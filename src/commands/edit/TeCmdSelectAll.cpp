#include "TeCmdSelectAll.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"

#include <QAbstractItemModel>

TeCmdSelectAll::TeCmdSelectAll()
{
}


TeCmdSelectAll::~TeCmdSelectAll()
{
}

/**
 * カレントのTeFileListView上での全選択/解除を実施する。
 */
bool TeCmdSelectAll::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileListView* p_list = p_folder->list();

		if (p_list->selectionModel()->hasSelection()) {
			//選択済みなら、全解除
			p_list->clearSelection();
		}
		else {
			//なにも選択されていなければ、全選択
			p_list->selectAll();
		}
	}

	return true;
}