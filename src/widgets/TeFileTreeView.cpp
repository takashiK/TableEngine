/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
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
