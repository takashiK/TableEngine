/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
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

TeFolderView * TeFileTreeView::folderView()
{
	return mp_folderView;
}

void TeFileTreeView::setFolderView(TeFolderView * view)
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
