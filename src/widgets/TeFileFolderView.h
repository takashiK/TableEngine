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

#include <QWidget>
#include <QPoint>
#include <QAbstractItemView>
#include "TeDispatchable.h"
#include "TeFolderView.h"
#include "utils/TeHistory.h"

/**
 * @file TeFileFolderView.h
 * @brief Two-pane folder view backed by QFileSystemModel.
 * @ingroup widgets
 *
 * @see doc/markdown/widgets/TeFileFolderView.md
 */

class TeFileTreeView;
class TeFileListView;
class QFileSystemModel;
class TeEventFilter;
class TeDispatcher;
class TeFileSortProxyModel;

/**
 * @class TeFileFolderView
 * @brief TeFolderView implementation for local file system navigation.
 * @ingroup widgets
 *
 * @details Hosts a TeFileTreeView (left pane) and a TeFileListView (right
 * pane).  Uses two independent QFileSystemModel instances — one for the tree
 * and one for the list — sorted via TeFileSortProxyModel.  Navigation history
 * is managed by a TeHistory instance.
 *
 * @see TeFolderView, TeFileTreeView, TeFileListView, TeHistory
 * @see doc/markdown/widgets/TeFileFolderView.md
 */
class TeFileFolderView : public TeFolderView
{
	Q_OBJECT

public:
	TeFileFolderView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileFolderView();

	/** @brief Returns TeTypes::WT_FILE_FOLDER_VIEW. */
	virtual TeTypes::WidgetType getType() const;

	/** @brief Returns the tree-view pane. */
	virtual TeFileTreeView* tree();
	/** @brief Returns the list-view pane. */
	virtual TeFileListView* list();

	/**
	 * @brief Sets the root path shown by the tree.
	 * @param path Absolute directory path.
	 */
	virtual void setRootPath(const QString& path);
	/** @brief Returns the current tree root path. */
	virtual QString rootPath();

	/**
	 * @brief Navigates the list view to @p path.
	 * @param path Absolute directory path inside the current root.
	 */
	virtual void setCurrentPath(const QString& path);
	/** @brief Returns the path currently displayed in the list view. */
	virtual QString currentPath();

	/** @brief Navigates forward in the history stack. */
	virtual void moveNextPath();
	/** @brief Navigates backward in the history stack. */
	virtual void movePrevPath();
	/** @brief Returns all paths in the navigation history. */
	virtual QStringList getPathHistory() const;

	/** @brief Creates and returns a TeFileFinder for this view. */
	virtual TeFinder* makeFinder();

public slots:
	/**
	 * @brief Updates file-type visibility flags and sort order.
	 * @param typeFlags     File type visibility flags.
	 * @param order         Sort column.
	 * @param orderReversed true for descending sort.
	 */
	virtual void setFileShowMode(TeTypes::FileTypeFlags typeFlags, TeTypes::OrderType order, bool orderReversed);

protected:
	/**
	 * @brief Updates both panes to reflect a new root/current combination.
	 * @param root    New tree root path.
	 * @param current New list-view current path (defaults to root).
	 */
	void updatePath(const QString& root, const QString& current=QString());

	/**
	 * @brief Shows a system context menu at @p pos for the item view @p pView.
	 * @param pView  The view that received the context-menu event.
	 * @param pos    Screen position for the menu.
	 */
	void showContextMenu(const QAbstractItemView* pView, const QPoint& pos);

	/**
	 * @brief Shows a user-defined context menu.
	 * @param menuName Name key identifying the menu definition in TeSettings.
	 * @param pos      Screen position for the menu.
	 */
	void showUserContextMenu(const QString& menuName, const QPoint& pos);

private:
	TeFileTreeView* mp_treeView;         ///< Left-pane tree widget.
	TeFileListView* mp_listView;         ///< Right-pane list widget.

	QFileSystemModel*     mp_treeModel;       ///< Model for the tree pane.
	QFileSystemModel*     mp_listModel;       ///< Model for the list pane.
	TeFileSortProxyModel* mp_listSortModel;   ///< Sort/filter proxy on the list model.

	TeEventFilter* mp_treeEvent;         ///< Event filter for the tree pane.
	TeEventFilter* mp_listEvent;         ///< Event filter for the list pane.

	TeHistory m_history;                 ///< Navigation history.
};
