#include "TeCmdFolderCloseUnder.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"


TeCmdFolderCloseUnder::TeCmdFolderCloseUnder()
{
}


TeCmdFolderCloseUnder::~TeCmdFolderCloseUnder()
{
}

bool TeCmdFolderCloseUnder::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		if (p_tree->currentIndex().isValid()) {
			p_tree->collapse(p_tree->currentIndex());
		}
	}

	return true;
}
