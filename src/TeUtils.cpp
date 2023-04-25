#include "TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"

#include <QStringList>
#include <QAbstractItemView>
#include <QFileSystemModel>

#include <QSet>
#include <QFileInfo>

#include <QDebug>

#include <magic.h>

namespace {
	const QSet<const QString> txtSuffixes{ "txt","html","htm","md","h","c","cpp","ini","py","json","ts","js","sh" };
	const QSet<const QString> imageSuffixes{ "jpg","jpeg","png","gif","bmp","tiff","svg" };
	const QSet<const QString> archiveSuffixes{ "zip","lzh","cab","7z","rar","tar","gz","bz2","xz","tgz","cpio" };
}

bool getSelectedItemList(TeViewStore* p_store, QStringList* p_paths)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

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
	return (p_paths != nullptr) && !p_paths->isEmpty();
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
