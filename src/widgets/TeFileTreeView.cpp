#include "TeFileTreeView.h"
#include <QEvent>

#include <QFileSystemModel>

TeFileTreeView::TeFileTreeView(QWidget *parent)
	: QTreeView(parent)
{
	mp_folderView = nullptr;
}

TeFileTreeView::~TeFileTreeView()
{
}

void TeFileTreeView::setVisualRootIndex(const QModelIndex & index)
{
	QModelIndex rootIndex = index.parent();
	if (m_rootIndex != index) {
		//非表示指定していたものを表示設定に戻す。
		for (int i = 0; i < model()->rowCount(m_rootIndex.parent()); i++) {
			setRowHidden(i, m_rootIndex.parent(), false);
		}

		m_rootIndex = index;
		m_rootIndexParent = rootIndex;
		for (int i = 0; i < model()->rowCount(rootIndex); i++) {
			if (index != model()->index(i, 0, rootIndex)) {
				setRowHidden(i, rootIndex, true);
			}
		}
	}
	QTreeView::setRootIndex(rootIndex);
	expand(m_rootIndex);
}

QModelIndex TeFileTreeView::visualRootIndex()
{
	return m_rootIndex;
}

TeFileFolderView * TeFileTreeView::folderView()
{
	return mp_folderView;
}

void TeFileTreeView::setFolderView(TeFileFolderView * view)
{
	mp_folderView = view;
}

void TeFileTreeView::rowsInserted(const QModelIndex & parent, int start, int end)
{
	if (parent == m_rootIndexParent && m_rootIndex.isValid()) {
		for (int i = start; i <= end; i++) {
			QModelIndex index = model()->index(i, 0, parent);
			if (index != m_rootIndex) {
				setRowHidden(i, parent, true);
			}
		}
	}
	QTreeView::rowsInserted(parent, start, end);
}
