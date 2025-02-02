#include "TeUtils.h"
#include "TeViewStore.h"
#include "TeSettings.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"

#include <QStringList>
#include <QAbstractItemView>
#include <QFileSystemModel>

#include <QSet>
#include <QFileInfo>

#include <QDebug>
#include <QSettings>

#include <magic.h>
#include <icu.h>


namespace {
	const QSet<const QString> txtSuffixes{ "txt","html","htm","md","h","c","cpp","ini","py","json","ts","js","sh" };
	const QSet<const QString> imageSuffixes{ "jpg","jpeg","png","gif","bmp","tiff","svg" };
	const QSet<const QString> archiveSuffixes{ "zip","lzh","cab","7z","rar","tar","gz","bz2","xz","tgz","cpio" };
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

		QFileSystemModel* model = qobject_cast<QFileSystemModel*>(p_itemView->model());

		if (model != nullptr) {
			if (p_itemView->selectionModel()->hasSelection()) {
				//target selected files.
				QModelIndexList indexList = p_itemView->selectionModel()->selectedIndexes();
				for (const QModelIndex& index : indexList)
				{
					if (index.column() == 0) {
						p_paths->append(model->filePath(index));
					}
				}
			}
			else {
				//no files selected. so use current file.
				if (p_itemView->currentIndex().isValid()) {
					p_paths->append(model->filePath(p_itemView->currentIndex()));
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

		QFileSystemModel* model = qobject_cast<QFileSystemModel*>(p_itemView->model());

		if (model != nullptr) {
			if (p_itemView->currentIndex().isValid()) {
				return model->filePath(p_itemView->currentIndex());
			}
		}
	}
	return QString();
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
	for (auto& key : settings.childKeys()) {
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
		settings.setValue(QString("path%1").arg(i, 2, 10, QChar('0')), list.at(i));
	}
	settings.endGroup();
}

bool isDir(const QAbstractItemModel* p_model, const QModelIndex& index)
{
	if (p_model != nullptr && index.isValid()) {
		const QFileSystemModel* model = qobject_cast<const QFileSystemModel*>(p_model);
		if (model != nullptr) {
			return model->isDir(index);
		}
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

	//check suffix
	if (fileInfo.isDir()) {
		type = TE_FILE_FOLDER;
	}
	else if (txtSuffixes.contains(fileInfo.suffix())) {
		type = TE_FILE_TEXT;
	}
	else if (imageSuffixes.contains(fileInfo.suffix())) {
		type = TE_FILE_IMAGE;
	}
	else if (archiveSuffixes.contains(fileInfo.suffix())) {
		type = TE_FILE_ARCHIVE;
	}

	if (type != TE_FILE_UNKNOWN){
		return type;
	}
	// Initialize the magic library
	static magic_t magic = nullptr;
	if (magic == nullptr) {
		magic = magic_open(MAGIC_MIME_TYPE);
		if (magic == nullptr) {
			qDebug() << "unable to initialize magic library";
			return TE_FILE_UNKNOWN;
		}
		if (magic_load(magic, nullptr) != 0) {
			qDebug() << "unable to load magic database - " << magic_error(magic);
			magic_close(magic);
			magic = nullptr;
			return TE_FILE_UNKNOWN;
		}
	}

	//get file type
	QString typeStr = magic_file(magic, path.toLocal8Bit().data());

	if (typeStr.startsWith("text/")) {
		type = TE_FILE_TEXT;
	}
	else if (typeStr.startsWith("image/")) {
		type = TE_FILE_IMAGE;
	}
	else if (typeStr.startsWith("application/zip")) {
		type = TE_FILE_ARCHIVE;
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
