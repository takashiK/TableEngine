#include "TeCmdFolderOpenAll.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"



TeCmdFolderOpenAll::TeCmdFolderOpenAll()
{
}


TeCmdFolderOpenAll::~TeCmdFolderOpenAll()
{
}

bool TeCmdFolderOpenAll::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		p_tree->expandAll();
	}

	return true;
}
