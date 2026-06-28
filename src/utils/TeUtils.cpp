#include "TeUtils.h"
#include "TeViewStore.h"
#include "TeSettings.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeArchiveFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"
#include "utils/TeFileInfo.h"
#include "utils/TeArchive.h"

#include <QStringList>
#include <QAbstractItemView>
#include <QFileSystemModel>

#include <QSet>
#include <QFileInfo>
#include <QDir>

#include <QDebug>
#include <QSettings>
#include <QMimeDatabase>

#include <icu.h>

/**
 * @file TeUtils.cpp
 * @brief Implementation of TeUtils.
 * @ingroup utility
 */

/**
 * @file TeUtils.cpp
 * @brief Implementation of TeUtils.
 * @ingroup utility
 */


namespace {
	const QSet<const QString> txtSuffixes{ "txt","html","htm","md","h","c","cpp","ini","py","json","ts","js","sh"};
	const QSet<const QString> imageSuffixes{ "jpg","jpeg","png","gif","bmp","tiff","svg" };
	const QSet<const QString> archiveSuffixes{ "zip","lzh","cab","7z","rar","tar","gz","bz2","xz","tgz","cpio" };

	// Resolves the path string for a model index, supporting both filesystem
	// items (QFileInfo via FileInfoRole) and archive items (virtual path via
	// TeFileInfo::ROLE_PATH).  Returns an empty string for non-selectable
	// synthetic entries such as "..".
	QString resolveItemPath(const QModelIndex& index)
	{
		QVariant var = index.data(QFileSystemModel::FileInfoRole);
		if (var.isValid() && var.canConvert<QFileInfo>()) {
			return qvariant_cast<QFileInfo>(var).filePath();
		}

		QVariant typeVar = index.data(TeFileInfo::ROLE_TYPE);
		if (typeVar.isValid()) {
			if (typeVar.toInt() == TeFileInfo::EN_PARENT)
				return QString();
			return index.data(TeFileInfo::ROLE_PATH).toString();
		}
		return QString();
	}
}

bool getSelectedItemList(TeViewStore* p_store, QStringList* p_paths)
{
	TeFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr && p_paths != nullptr) {
		//get selected files
		QAbstractItemView* p_itemView = nullptr;
		if (p_folder->tree()->hasFocus()) {
			p_itemView = p_folder->tree();
		}
		else {
			p_itemView = p_folder->list();
		}

		if (p_itemView->selectionModel()->hasSelection()) {
			//target selected files.
			QModelIndexList indexList = p_itemView->selectionModel()->selectedIndexes();
			for (const QModelIndex& index : indexList)
			{
				if (index.column() == 0) {
					QString path = resolveItemPath(index);
					if (!path.isEmpty()) {
						p_paths->append(path);
					}
				}
			}
		}
		else {
			//no files selected. so use current file.
			if (p_itemView->currentIndex().isValid()) {
				QString path = resolveItemPath(p_itemView->currentIndex());
				if (!path.isEmpty()) {
					p_paths->append(path);
				}
			}
		}
	}
	return (p_paths != nullptr) && !p_paths->isEmpty();
}

QString getCurrentItem(TeViewStore* p_store)
{
	TeFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr ) {
		//get selected files
		QAbstractItemView* p_itemView = nullptr;
		if (p_folder->tree()->hasFocus()) {
			p_itemView = p_folder->tree();
		}
		else {
			p_itemView = p_folder->list();
		}

		if (p_itemView->currentIndex().isValid()) {
			return resolveItemPath(p_itemView->currentIndex());
		}
	}
	return QString();
}

QStringList extractArchiveSelection(TeViewStore* p_store, const QStringList& entries)
{
	QStringList result;

	TeArchiveFolderView* p_arc = qobject_cast<TeArchiveFolderView*>(p_store->currentFolderView());
	if (p_arc == nullptr || p_arc->reader() == nullptr) {
		return result;
	}

	QString tempDir = p_arc->tempPath();
	if (tempDir.isEmpty()) {
		return result;
	}

	TeArchive::Reader* reader = p_arc->reader();
	reader->clearCancel();
	if (!reader->extract(tempDir, QString(), entries)) {
		return result;
	}

	const QString base = tempDir.endsWith('/') ? tempDir : tempDir + "/";
	for (const auto& entry : entries) {
		result.append(QDir::cleanPath(base + entry));
	}
	return result;
}

QString getCurrentFolder(TeViewStore* p_store)
{
	TeFolderView* p_folder = p_store->currentFolderView();
	if (p_folder != nullptr) {
		return p_folder->currentPath();
	}
	return QString();
}

QStringList getFavorites()
{
	QSettings settings;
	QStringList favorites;
	settings.beginGroup(SETTING_FAVORITES);
	for (const auto& key : settings.childKeys()) {
		favorites.append(settings.value(key).toString());
	}
	settings.endGroup();
	return favorites;
}

void updateFavorites(const QStringList& list)
{
	QSettings settings;
	settings.beginGroup(SETTING_FAVORITES);
	settings.remove("");
	for (int i = 0; i < list.size(); i++) {
		settings.setValue(QString("path%1").arg(i, 2, 10, u'0'), list.at(i));
	}
	settings.endGroup();
}

bool isDir(const QModelIndex& index)
{
	if (index.isValid()) {
		QVariant var = index.data(QFileSystemModel::FileInfoRole);
		Q_ASSERT(var.isValid() && var.canConvert<QFileInfo>());
		QFileInfo fileInfo = qvariant_cast<QFileInfo>(var);
		return fileInfo.isDir();
	}
	return false;
}

/*!
 * \brief Get file type.
 * \param path file path.
 * \return file type.
 * 
 * Analyze file type by suffix and libmagic.
 */
TeFileType getFileType(const QString& path)
{
	TeFileType type = TE_FILE_UNKNOWN;
	QFileInfo fileInfo(path);

	//lazy check by suffix
	if (fileInfo.isDir()) {
		type = TE_FILE_FOLDER;
	} else if (txtSuffixes.contains(fileInfo.suffix().toLower())) {
		type = TE_FILE_TEXT;
	}
	else if (imageSuffixes.contains(fileInfo.suffix().toLower())) {
		type = TE_FILE_IMAGE;
	}
	else if (archiveSuffixes.contains(fileInfo.suffix().toLower())) {
		type = TE_FILE_ARCHIVE;
	}

	if (type != TE_FILE_UNKNOWN){
		return type;
	}

	if (fileInfo.isFile() && fileInfo.size() > 0) {
		QMimeDatabase db;

		QMimeType mime = db.mimeTypeForFile(fileInfo);
		QString typeStr = mime.name();
		if (mime.inherits("text/plain")) {
			// The file is plain text, we can display it in a QTextEdit
		}

		if (typeStr.startsWith("text/")) {
			type = TE_FILE_TEXT;
		}
		else if (typeStr.startsWith("image/")) {
			type = TE_FILE_IMAGE;
		}
		else if (typeStr.startsWith("application/zip")) {
			type = TE_FILE_ARCHIVE;
		}
	}

	return type;
}


/*!
*  \brief Detect text codec from data uing ICU
*  \param data Data to detect
*  \param codecList List of codec names to detect
*  \return The name of the codec
*/
QString detectTextCodec(const QByteArray& data, const QStringList& codecList) {
	const char* const kDefaultCodec = "UTF-8";

	UErrorCode status = U_ZERO_ERROR;
	UCharsetDetector* detector = ucsdet_open(&status);
	if (U_FAILURE(status)) {
		return QString(kDefaultCodec);
	}
	ucsdet_setText(detector, data.constData(), data.size(), &status);
	if (U_FAILURE(status)) {
		ucsdet_close(detector);
		return QString(kDefaultCodec);
	}

	int count = 0;
	const UCharsetMatch** matchs = ucsdet_detectAll(detector, &count, &status);
	for (int i = 0; i < count; i++) {
		const char* name = ucsdet_getName(matchs[i], &status);
		if (U_FAILURE(status)) {
			ucsdet_close(detector);
			return QString(kDefaultCodec);
		}
		if (codecList.contains(name)) {
			ucsdet_close(detector);
			return QString::fromUtf8(name);
		}
	}

	ucsdet_close(detector);
	return QString(kDefaultCodec);
}
