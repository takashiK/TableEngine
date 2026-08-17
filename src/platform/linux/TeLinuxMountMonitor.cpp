#include "TeLinuxMountMonitor.h"

#include "platform/TeNativeEvent.h"

#include <QFile>
#include <QSocketNotifier>

#include <fcntl.h>
#include <unistd.h>

namespace {
constexpr int kDebounceIntervalMs = 300;
}

TeLinuxMountMonitor::TeLinuxMountMonitor(TeNativeEvent* nativeEvent, QObject* parent)
	: QObject(parent)
	, m_nativeEvent(nativeEvent)
{
	m_debounceTimer.setSingleShot(true);
	m_debounceTimer.setInterval(kDebounceIntervalMs);
	connect(&m_debounceTimer, &QTimer::timeout, this, [this] { checkProjection(); });
}

TeLinuxMountMonitor::~TeLinuxMountMonitor()
{
    if (m_mountInfoNotifier != nullptr) {
        delete m_mountInfoNotifier;
        m_mountInfoNotifier = nullptr;
    }
	if (m_mountInfoFd >= 0) {
		::close(m_mountInfoFd);
	}
}

void TeLinuxMountMonitor::start()
{
	if (m_mountInfoFd >= 0) {
		return;
	}

	m_mountInfoFd = ::open("/proc/self/mountinfo", O_RDONLY | O_CLOEXEC | O_NONBLOCK);
	if (m_mountInfoFd < 0) {
		return;
	}

	m_mountInfoNotifier = new QSocketNotifier(m_mountInfoFd, QSocketNotifier::Exception, this);
	connect(m_mountInfoNotifier, &QSocketNotifier::activated, this,
		[this] { handleMountInfoNotification(); });
	m_mountInfo = readMountInfo();
	m_snapshot = getDriveActions();
}

void TeLinuxMountMonitor::handleMountInfoNotification()
{
	scheduleProjectionCheck();
}

void TeLinuxMountMonitor::scheduleProjectionCheck()
{
	if (!m_debounceTimer.isActive()) {
		m_debounceTimer.start();
	}
}

void TeLinuxMountMonitor::checkProjection()
{
	const QByteArray mountInfo = readMountInfo();
	if (mountInfo == m_mountInfo) {
		return;
	}
	m_mountInfo = mountInfo;
	const QList<TeDriveAction> current = getDriveActions();
	const platform_util_test::DriveActionChange change =
		platform_util_test::compareDriveActionSnapshots(m_snapshot, current);
	m_snapshot = current;
	if (change.changed && m_nativeEvent != nullptr) {
		m_nativeEvent->changeMountState(change.state);
	}
}
QByteArray TeLinuxMountMonitor::readMountInfo() const
{
	QFile mountInfo(QStringLiteral("/proc/self/mountinfo"));
	if (!mountInfo.open(QIODevice::ReadOnly)) {
		return {};
	}
	return mountInfo.readAll();
}