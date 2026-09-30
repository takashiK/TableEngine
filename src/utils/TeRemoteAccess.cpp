#include "TeRemoteAccess.h"

#include <QDir>
#include <QHash>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>

#include <array>
#include <atomic>
#include <chrono>

#ifdef Q_OS_WIN
#include <windows.h>
#include <winnetwk.h>
#endif

/**
 * @file TeRemoteAccess.cpp
 * @brief Implementation of the TeRemoteAccess policy helpers.
 * @ingroup utility
 */

namespace TeRemoteAccess {

namespace {

constexpr int kDefaultMode = TeSettings::REMOTE_ACCESS_SUPPRESS_ICON;

std::array<std::atomic<int>, KIND_COUNT>& modeTable()
{
	static std::array<std::atomic<int>, KIND_COUNT> table = {{
		{kDefaultMode}, {kDefaultMode}, {kDefaultMode}, {kDefaultMode}
	}};
	return table;
}

QMutex& hookMutex()
{
	static QMutex m;
	return m;
}

std::function<Kind(const QString&)>& hook()
{
	static std::function<Kind(const QString&)> h;
	return h;
}

std::atomic<bool> g_hookActive{false};

bool isWslHost(const QString& host)
{
	return host.compare(QLatin1String("wsl$"), Qt::CaseInsensitive) == 0
		|| host.compare(QLatin1String("wsl.localhost"), Qt::CaseInsensitive) == 0;
}

QString uncHost(const QString& remoteName)
{
	QString p = QDir::fromNativeSeparators(remoteName);
	while (p.startsWith(QLatin1Char('/')))
		p.remove(0, 1);
	const int slash = p.indexOf(QLatin1Char('/'));
	return slash < 0 ? p : p.left(slash);
}

constexpr unsigned kDriveRemote = 4;          // DRIVE_REMOTE
constexpr unsigned kErrorConnectionUnavail = 1201; // ERROR_CONNECTION_UNAVAIL

struct UncertainDrive {
	Kind kind = Kind::Local;
	qint64 expiresAtMs = 0;
};

QMutex g_driveMutex;
QHash<QChar, Kind> g_driveCache;
QHash<QChar, UncertainDrive> g_driveUncertain;

QMutex g_oneDriveMutex;
QStringList g_oneDriveRoots;
bool g_oneDriveRootsValid = false;

#ifdef Q_OS_WIN
QStringList discoverOneDriveRoots()
{
	QStringList roots;
	for (const char* name : {"OneDrive", "OneDriveConsumer", "OneDriveCommercial"}) {
		const QString value = qEnvironmentVariable(name);
		if (!value.isEmpty())
			roots.append(value);
	}

	// Account sync roots registered by the OneDrive client (personal / business).
	QSettings accounts(QStringLiteral("HKEY_CURRENT_USER\\Software\\Microsoft\\OneDrive\\Accounts"), QSettings::NativeFormat);
	for (const QString& account : accounts.childGroups()) {
		const QString folder = accounts.value(account + QLatin1String("/UserFolder")).toString();
		if (!folder.isEmpty() && !roots.contains(folder, Qt::CaseInsensitive))
			roots.append(folder);
	}
	return roots;
}

QStringList oneDriveRoots()
{
	QMutexLocker lock(&g_oneDriveMutex);
	if (!g_oneDriveRootsValid) {
		g_oneDriveRoots = discoverOneDriveRoots();
		g_oneDriveRootsValid = true;
	}
	return g_oneDriveRoots;
}

DriveResolution probeDriveOs(QChar letter)
{
	const wchar_t root[] = { static_cast<wchar_t>(letter.unicode()), L':', L'\\', 0 };
	const unsigned driveType = GetDriveTypeW(root);

	QString remoteName;
	unsigned connectionResult = NO_ERROR;
	if (driveType == DRIVE_REMOTE || driveType == DRIVE_NO_ROOT_DIR || driveType == DRIVE_UNKNOWN) {
		const wchar_t local[] = { static_cast<wchar_t>(letter.unicode()), L':', 0 };
		wchar_t remote[MAX_PATH] = {};
		DWORD length = MAX_PATH;
		connectionResult = WNetGetConnectionW(local, remote, &length);
		remoteName = QString::fromWCharArray(remote);
	}

	return resolveDriveProbe(driveType, connectionResult, remoteName);
}

Kind resolveDriveOs(QChar letter)
{
	const qint64 nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count();
	return resolveDriveCached(letter, probeDriveOs, nowMs);
}
#endif

} // anonymous namespace

Kind resolveDriveCached(QChar letter, const std::function<DriveResolution(QChar)>& probe, qint64 nowMs)
{
	{
		QMutexLocker lock(&g_driveMutex);
		const auto it = g_driveCache.constFind(letter);
		if (it != g_driveCache.constEnd())
			return it.value();
		const auto uncertain = g_driveUncertain.constFind(letter);
		if (uncertain != g_driveUncertain.constEnd() && nowMs < uncertain->expiresAtMs)
			return uncertain->kind;
	}

	const DriveResolution resolution = probe(letter);
	QMutexLocker lock(&g_driveMutex);
	if (resolution.cacheable) {
		g_driveCache.insert(letter, resolution.kind);
		g_driveUncertain.remove(letter);
	}
	else {
		g_driveUncertain.insert(letter, {resolution.kind, nowMs + DRIVE_UNCERTAIN_TTL_MS});
	}
	return resolution.kind;
}

DriveResolution resolveDriveProbe(unsigned driveType, unsigned connectionResult, const QString& remoteName)
{
	DriveResolution result;
	const bool mapped = connectionResult == 0 || connectionResult == kErrorConnectionUnavail;

	if (mapped && (driveType == kDriveRemote || driveType == 0 || driveType == 1)) {
		result.kind = isWslHost(uncHost(remoteName)) ? Kind::Wsl : Kind::Network;
		// A disconnected mapping or a non-remote drive type may change later.
		result.cacheable = connectionResult == 0 && driveType == kDriveRemote;
		return result;
	}

	if (driveType == kDriveRemote) {
		// Remote but the mapping could not be read: Network is a guess, do not remember it.
		result.kind = Kind::Network;
		result.cacheable = false;
		return result;
	}

	result.kind = Kind::Local;
	// Unknown / missing root may be a drive that appears later.
	result.cacheable = driveType != 0 && driveType != 1;
	return result;
}

bool isLexicalRoot(const QString& path)
{
	const QString p = QDir::fromNativeSeparators(path);
	if (p == QLatin1String("/"))
		return true;
	if (p.size() >= 2 && p.size() <= 3 && p.at(0).isLetter() && p.at(1) == QLatin1Char(':'))
		return p.size() == 2 || p.at(2) == QLatin1Char('/');
	if (p.startsWith(QLatin1String("//")) && p.size() > 2 && p.at(2) != QLatin1Char('/')) {
		const QStringList parts = p.mid(2).split(QLatin1Char('/'), Qt::SkipEmptyParts);
		return parts.size() <= 2;
	}
	return false;
}

Kind classifyPath(const QString& path, const DriveKindResolver& resolveDrive, const QStringList& oneDriveRoots)
{
	QString p = QDir::fromNativeSeparators(path);

	if (p.startsWith(QLatin1String("//?/UNC/"), Qt::CaseInsensitive)) {
		p = QLatin1String("//") + p.mid(8);
	}
	else if (p.startsWith(QLatin1String("//?/")) || p.startsWith(QLatin1String("//./"))) {
		p = p.mid(4);
	}

	if (p.startsWith(QLatin1String("//")) && p.size() > 2 && p.at(2) != QLatin1Char('/')) {
		return isWslHost(uncHost(p)) ? Kind::Wsl : Kind::Network;
	}

	if (p.size() >= 2 && p.at(1) == QLatin1Char(':') && p.at(0).isLetter() && resolveDrive) {
		const Kind driveKind = resolveDrive(p.at(0).toUpper());
		if (driveKind != Kind::Local)
			return driveKind;
	}

	for (const QString& root : oneDriveRoots) {
		QString r = QDir::fromNativeSeparators(root);
		while (r.size() > 1 && r.endsWith(QLatin1Char('/')))
			r.chop(1);
		if (r.isEmpty())
			continue;
		if (p.compare(r, Qt::CaseInsensitive) == 0
			|| p.startsWith(r + QLatin1Char('/'), Qt::CaseInsensitive)) {
			return Kind::OneDrive;
		}
	}

	return Kind::Local;
}

Kind classify(const QString& path)
{
	if (g_hookActive.load(std::memory_order_acquire)) {
		std::function<Kind(const QString&)> copy;
		{
			QMutexLocker lock(&hookMutex());
			copy = hook();
		}
		if (copy)
			return copy(path);
	}

#ifdef Q_OS_WIN
	return classifyPath(path, resolveDriveOs, oneDriveRoots());
#else
	return Kind::Local;
#endif
}

void clearDriveCache()
{
	{
		QMutexLocker lock(&g_driveMutex);
		g_driveCache.clear();
		g_driveUncertain.clear();
	}
	QMutexLocker lock(&g_oneDriveMutex);
	g_oneDriveRootsValid = false;
}

TeSettings::RemoteAccessMode defaultMode()
{
	return static_cast<TeSettings::RemoteAccessMode>(kDefaultMode);
}

TeSettings::RemoteAccessMode mode(Kind kind)
{
	if (kind == Kind::Local)
		return TeSettings::REMOTE_ACCESS_NORMAL;
	return static_cast<TeSettings::RemoteAccessMode>(modeTable()[static_cast<size_t>(kind)].load(std::memory_order_relaxed));
}

void setMode(Kind kind, TeSettings::RemoteAccessMode value)
{
	if (kind == Kind::Local)
		return;
	modeTable()[static_cast<size_t>(kind)].store(value, std::memory_order_relaxed);
}

void resetModes()
{
	for (Kind k : {Kind::Wsl, Kind::Network, Kind::OneDrive})
		setMode(k, defaultMode());
}

const char* settingKey(Kind kind)
{
	switch (kind) {
	case Kind::Wsl:      return SETTING_FOLDER_REMOTE_WSL;
	case Kind::Network:  return SETTING_FOLDER_REMOTE_NETWORK;
	case Kind::OneDrive: return SETTING_FOLDER_REMOTE_ONEDRIVE;
	case Kind::Local:    break;
	}
	return nullptr;
}

bool loadFromSettings()
{
	QSettings settings;
	bool changed = false;
	for (Kind k : {Kind::Wsl, Kind::Network, Kind::OneDrive}) {
		const int raw = settings.value(settingKey(k), kDefaultMode).toInt();
		const int bounded = qBound(int(TeSettings::REMOTE_ACCESS_SUPPRESS_ICON), raw, int(TeSettings::REMOTE_ACCESS_NORMAL));
		const auto value = static_cast<TeSettings::RemoteAccessMode>(bounded);
		if (mode(k) != value) {
			setMode(k, value);
			changed = true;
		}
	}
	clearDriveCache();
	return changed;
}

TeSettings::RemoteAccessMode modeForPath(const QString& path)
{
	return mode(classify(path));
}

bool suppressIconAccess(const QString& path)
{
	return modeForPath(path) != TeSettings::REMOTE_ACCESS_NORMAL;
}

bool suppressAutoContent(const QString& path)
{
	return modeForPath(path) == TeSettings::REMOTE_ACCESS_SUPPRESS_AUTO;
}

void setClassifierForTesting(std::function<Kind(const QString&)> classifier)
{
	QMutexLocker lock(&hookMutex());
	hook() = std::move(classifier);
	g_hookActive.store(static_cast<bool>(hook()), std::memory_order_release);
}

} // namespace TeRemoteAccess
