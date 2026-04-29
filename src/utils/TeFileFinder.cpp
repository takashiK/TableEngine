#include "TeFileFinder.h"

#include <QDir>
#include <QFileInfo>

/**
 * @file TeFileFinder.cpp
 * @brief Implementation of TeFileFinder.
 * @ingroup utility
 */

/**
 * @file TeFileFinder.cpp
 * @brief Implementation of TeFileFinder.
 * @ingroup utility
 */

TeFileFinder::TeFileFinder(const QString& path, QObject* parent)
	: TeFinder(parent)
	, m_rootPath(path)
{}

TeFileFinder::~TeFileFinder()
{}

bool TeFileFinder::isValid() const
{
	return QDir(m_rootPath).exists();
}

bool TeFileFinder::matchesQuery(const QFileInfo& info, const TeSearchQuery& query) const
{
	if (query.hasNameFilter) {
		if (!query.namePattern.match(info.fileName()).hasMatch())
			return false;
	}

	if (query.hasSizeFilter && info.isFile()) {
		if (query.sizeLessThan) {
			if (info.size() > query.sizeThreshold) return false;
		} else {
			if (info.size() < query.sizeThreshold) return false;
		}
	}

	if (query.hasDateFilter) {
		const QDateTime modified = info.lastModified();
		if (modified < query.dateFrom || modified > query.dateTo)
			return false;
	}

	return true;
}

void TeFileFinder::doSearch(const TeSearchQuery& query)
{
	QStringList pendingDirs;
	pendingDirs.append(query.targetPath);

	while (!pendingDirs.isEmpty() && !isCancelled()) {
		const QString dirPath = pendingDirs.takeFirst();
		QDir dir(dirPath);

		QList<TeFileInfo> batch;
		const auto entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
		for (const QFileInfo& info : entries) {
			if (isCancelled()) break;

			if (info.isDir() && query.recursive) {
				pendingDirs.append(info.filePath());
			}

			if (matchesQuery(info, query)) {
				TeFileInfo teInfo;
				teInfo.path         = info.filePath();
				teInfo.size         = info.size();
				teInfo.lastModified = info.lastModified();
				teInfo.permissions  = static_cast<int>(info.permissions());
				teInfo.type         = info.isDir()  ? TeFileInfo::EN_DIR
				                    : info.isFile() ? TeFileInfo::EN_FILE
				                    : TeFileInfo::EN_NONE;
				batch.append(teInfo);
			}
		}

		if (!batch.isEmpty()) {
			reportItems(batch);
		}
	}
}
