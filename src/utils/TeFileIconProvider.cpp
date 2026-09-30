#include "TeFileIconProvider.h"
#include "TeRemoteAccess.h"

#include <QCoreApplication>
#include <QFileIconProvider>
#include <QFileInfo>

/**
 * @file TeFileIconProvider.cpp
 * @brief Implementation of TeFileIconProvider.
 * @ingroup utility
 */

namespace {
TeFileIconProvider* g_instance = nullptr;
TeFileIconProvider* g_folderInstance = nullptr;
}

TeFileIconProvider::TeFileIconProvider(std::unique_ptr<QAbstractFileIconProvider> fallback, bool foldersOnly)
	: m_foldersOnly(foldersOnly)
	, mp_fallback(fallback ? std::move(fallback) : std::make_unique<QFileIconProvider>())
	, m_folder(mp_fallback->icon(QAbstractFileIconProvider::Folder))
	, m_file(mp_fallback->icon(QAbstractFileIconProvider::File))
	, m_drive(mp_fallback->icon(QAbstractFileIconProvider::Drive))
{
}

TeFileIconProvider::~TeFileIconProvider() = default;

QIcon TeFileIconProvider::icon(IconType type) const
{
	return mp_fallback->icon(type);
}

QIcon TeFileIconProvider::icon(const QFileInfo& info) const
{
	if (!TeRemoteAccess::suppressIconAccess(info.filePath()))
		return mp_fallback->icon(info);

	if (TeRemoteAccess::isLexicalRoot(info.filePath()))
		return m_drive;
	return (m_foldersOnly || info.isDir()) ? m_folder : m_file;
}

QString TeFileIconProvider::type(const QFileInfo& info) const
{
	if (!TeRemoteAccess::suppressIconAccess(info.filePath()))
		return mp_fallback->type(info);

	if (TeRemoteAccess::isLexicalRoot(info.filePath()))
		return QCoreApplication::translate("TeFileIconProvider", "Drive");
	if (info.isDir())
		return QCoreApplication::translate("TeFileIconProvider", "Folder");
	const QString suffix = info.suffix();
	return suffix.isEmpty()
		? QCoreApplication::translate("TeFileIconProvider", "File")
		: QCoreApplication::translate("TeFileIconProvider", "%1 File").arg(suffix.toUpper());
}

QIcon TeFileIconProvider::folderIcon(const QString& path) const
{
	if (!TeRemoteAccess::suppressIconAccess(path))
		return mp_fallback->icon(QFileInfo(path));

	return TeRemoteAccess::isLexicalRoot(path) ? m_drive : m_folder;
}

TeFileIconProvider* TeFileIconProvider::instance()
{
	if (!g_instance) {
		g_instance = new TeFileIconProvider;
		qAddPostRoutine([] {
			delete g_instance;
			g_instance = nullptr;
		});
	}
	return g_instance;
}

TeFileIconProvider* TeFileIconProvider::folderInstance()
{
	if (!g_folderInstance) {
		g_folderInstance = new TeFileIconProvider(nullptr, true);
		qAddPostRoutine([] {
			delete g_folderInstance;
			g_folderInstance = nullptr;
		});
	}
	return g_folderInstance;
}
