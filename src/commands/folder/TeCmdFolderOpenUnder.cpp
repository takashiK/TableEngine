#include "TeCmdFolderOpenUnder.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"

#include <QFileSystemModel>
#include <QModelIndex>

TeCmdFolderOpenUnder::TeCmdFolderOpenUnder()
{
}


TeCmdFolderOpenUnder::~TeCmdFolderOpenUnder()
{
}

bool TeCmdFolderOpenUnder::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		if (p_tree->currentIndex().isValid()) {
			p_tree->expand(p_tree->currentIndex());
			if(p_tree->model()->hasChildren(p_tree->currentIndex()))
				openUnder(p_tree, p_tree->currentIndex());
		}
	}

	return true;
}

void TeCmdFolderOpenUnder::openUnder(TeFileTreeView* p_tree, const QModelIndex& index)
{
	QAbstractItemModel* model = p_tree->model();
	if (model->hasChildren(index)) {
		p_tree->expand(index);
		for (int i = 0; i < model->rowCount(index); i++) {
			QModelIndex child = model->index(i, 0, index);
			if(model->hasChildren(child))
				openUnder(p_tree, child);
		}
	}
}
