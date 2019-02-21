#include "TeCmdFolderOpenOne.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"



TeCmdFolderOpenOne::TeCmdFolderOpenOne()
{
}


TeCmdFolderOpenOne::~TeCmdFolderOpenOne()
{
}

bool TeCmdFolderOpenOne::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		if (p_tree->currentIndex().isValid()) {
			p_tree->expand(p_tree->currentIndex());
		}
	}

	return true;
}
