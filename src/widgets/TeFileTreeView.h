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

#pragma once

#include <QTreeView>
#include <QList>

/**
 * @file TeFileTreeView.h
 * @brief Directory tree-view widget used in the left pane of TeFolderView.
 * @ingroup widgets
 */

class TeFolderView;

/**
 * @class TeFileTreeView
 * @brief QTreeView subclass providing a directory navigation tree.
 * @ingroup widgets
 *
 * @details Extends QTreeView with the ability to set an independent visual
 * root that differs from the model's root index, allowing the tree to show
 * only a sub-tree of the underlying file-system model.
 *
 * Each instance holds a back-reference to its owning TeFolderView, which is
 * used by TeEventFilter to identify the dispatch target.
 */
class TeFileTreeView : public QTreeView
{
	Q_OBJECT

public:
	TeFileTreeView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileTreeView();

	/**
	 * @brief Sets the visual root index independently of the model root.
	 *
	 * All items above @p index are hidden, making @p index appear as the
	 * top-level node.  The model's root index is left unchanged.
	 * @param index The model index to use as the displayed root.
	 */
	void	setVisualRootIndex(const QModelIndex &index);

	/** @brief Returns the current visual root model index. */
	QModelIndex visualRootIndex();

	/** @brief Returns the owning TeFolderView. */
	virtual TeFolderView* folderView();

	/**
	 * @brief Sets the owning TeFolderView back-reference.
	 * @param view The parent TeFolderView.
	 */
	void setFolderView(TeFolderView* view);

protected slots:
	/**
	 * @brief Expands newly inserted rows to maintain visual root behaviour.
	 * @param parent The parent model index.
	 * @param start  First inserted row.
	 * @param end    Last inserted row.
	 */
	virtual void	rowsInserted(const QModelIndex &parent, int start, int end);

private:
	QModelIndex m_rootIndex;
	QModelIndex m_rootIndexParent;
	TeFolderView* mp_folderView;
};
