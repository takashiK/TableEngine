#include "TeArchiveFinder.h"
#include "TeArchive.h"

#include <QFileInfo>

/**
 * @file TeArchiveFinder.cpp
 * @brief Implementation of TeArchiveFinder.
 * @ingroup utility
 */

/**
 * @file TeArchiveFinder.cpp
 * @brief Implementation of TeArchiveFinder.
 * @ingroup utility
 */

TeArchiveFinder::TeArchiveFinder(const QString& path, QObject* parent)
	: TeFinder(parent)
{
	auto* reader = new TeArchive::Reader();
	if (reader->open(path)) {
		mp_reader = reader;
	} else {
		delete reader;
	}
}

TeArchiveFinder::~TeArchiveFinder()
{
	delete mp_reader;
}

bool TeArchiveFinder::isValid() const
{
	return mp_reader != nullptr;
}

bool TeArchiveFinder::matchesEntry(const TeFileInfo& entry, const TeSearchQuery& query) const
{
	if (query.hasNameFilter) {
		const QFileInfo fi(entry.path);
		if (!query.namePattern.match(fi.fileName()).hasMatch())
			return false;
	}

	if (query.hasSizeFilter && entry.type == TeFileInfo::EN_FILE) {
		if (query.sizeLessThan) {
			if (entry.size > query.sizeThreshold) return false;
		} else {
			if (entry.size < query.sizeThreshold) return false;
		}
	}

	if (query.hasDateFilter) {
		if (entry.lastModified < query.dateFrom || entry.lastModified > query.dateTo)
			return false;
	}

	return true;
}

void TeArchiveFinder::doSearch(const TeSearchQuery& query)
{
	if (mp_reader == nullptr) return;

	// Group entries by their parent directory path to report folder-by-folder.
	QString           currentDir;
	QList<TeFileInfo> batch;

	for (const TeFileInfo& entry : *mp_reader) {
		if (isCancelled()) break;

		if (!entry.path.startsWith(query.targetPath)) continue;

		const QString parentDir = QFileInfo(entry.path).path();
		if (parentDir != currentDir) {
			if (!batch.isEmpty()) {
				reportItems(batch);
				batch.clear();
			}
			currentDir = parentDir;
		}

		if (matchesEntry(entry, query)) {
			batch.append(entry);
		}
	}

	if (!batch.isEmpty() && !isCancelled()) {
		reportItems(batch);
	}
}

