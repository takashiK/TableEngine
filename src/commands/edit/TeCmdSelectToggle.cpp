#include "TeCmdSelectToggle.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"



TeCmdSelectToggle::TeCmdSelectToggle()
{
}


TeCmdSelectToggle::~TeCmdSelectToggle()
{
}

/**
* カレントのTeFileListView上での選択状態を反転する。
*/
bool TeCmdSelectToggle::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileListView* p_list = p_folder->list();
		QAbstractItemModel* model = p_list->model();
		QModelIndex root = p_list->rootIndex();

		QModelIndex startIndex = model->index(0, 0, root);
		QModelIndex endIndex = model->index(model->rowCount(root) -1, model->columnCount(root)-1, root);

		QItemSelection selection(startIndex, endIndex);
		p_list->selectionModel()->select(selection, QItemSelectionModel::Toggle);
	}

	return true;
}
