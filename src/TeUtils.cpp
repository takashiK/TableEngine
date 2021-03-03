#include "TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"

#include <QStringList>
#include <QAbstractItemView>
#include <QFileSystemModel>


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
