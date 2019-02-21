#include "TeCmdFolderCloseAll.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"


TeCmdFolderCloseAll::TeCmdFolderCloseAll()
{
}


TeCmdFolderCloseAll::~TeCmdFolderCloseAll()
{
}

bool TeCmdFolderCloseAll::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		if (p_tree->currentIndex().isValid()) {
			p_tree->collapseAll();
		}
	}

	return true;
}
