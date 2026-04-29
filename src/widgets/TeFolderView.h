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
#include "TeDispatchable.h"

/**
 * @file TeFolderView.h
 * @brief Abstract base class for all folder-view widgets.
 * @ingroup widgets
 *
 * @details Declares TeFolderView, which defines the two-pane navigation
 * interface shared by TeFileFolderView, TeArchiveFolderView, and
 * TeFindFolderView.
 *
 * @see doc/markdown/widgets/TeFolderView.md
 */

class TeFileTreeView;
class TeFileListView;
class TeFinder;

/**
 * @class TeFolderView
 * @brief Abstract two-pane folder-view widget.
 * @ingroup widgets
 *
 * @details Provides a unified navigation interface (setRootPath, setCurrentPath,
 * moveNextPath, movePrevPath) and acts as an event-dispatch bridge between
 * TeEventFilter and the application-wide TeDispatcher.  Concrete subclasses
 * implement the navigation interface for different data sources.
 *
 * @see TeFileFolderView, TeArchiveFolderView, TeFindFolderView
 * @see doc/markdown/widgets/TeFolderView.md
 */
class TeFolderView : public QWidget, public TeDispatchable
{
	Q_OBJECT

public:
	TeFolderView(QWidget *parent);
	~TeFolderView();

	/** @brief Returns the tree-view pane for this folder view. */
	virtual TeFileTreeView* tree() = 0;

	/** @brief Returns the list-view pane for this folder view. */
	virtual TeFileListView* list() = 0;

	/**
	 * @brief Sets the dispatcher used for event-to-command routing.
	 * @param p_dispatcher The application-wide dispatcher.
	 */
	virtual void setDispatcher(TeDispatchable* p_dispatcher);

	/**
	 * @brief Filters and forwards widget events to the dispatcher.
	 *
	 * Only key events matching isDispatchable() criteria are forwarded;
	 * all others are silently accepted.
	 */
	virtual bool dispatch(TeTypes::WidgetType type, QEvent *event);
	virtual void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

	/**
	 * @brief Sets the root path displayed at the top of the tree.
	 * @param path The new root path.
	 */
	virtual void setRootPath(const QString& path) = 0;

	/** @brief Returns the current root path. */
	virtual QString rootPath() =0;

	/**
	 * @brief Navigates the list view to @p path within the current root.
	 * @param path The target directory path.
	 */
	virtual void setCurrentPath(const QString& path) =0;

	/** @brief Returns the path currently shown in the list view. */
	virtual QString currentPath() =0;

	/** @brief Navigates to the next path in the history stack. */
	virtual void moveNextPath() = 0;

	/** @brief Navigates to the previous path in the history stack. */
	virtual void movePrevPath() = 0;

	/** @brief Returns the navigation history as a list of path strings. */
	virtual QStringList getPathHistory() const = 0;

	/**
	 * @brief Creates and returns the appropriate TeFinder for this view type.
	 *
	 * TeFileFolderView returns a TeFileFinder; TeArchiveFolderView returns
	 * a TeArchiveFinder; TeFindFolderView returns nullptr.
	 * @return Heap-allocated finder; ownership transferred to caller.
	 */
	virtual TeFinder* makeFinder() = 0;

public slots:
	/**
	 * @brief Updates the visible file types and sort order.
	 * @param typeFlags     Which file types to show (hidden, system, etc.).
	 * @param order         Sort criterion.
	 * @param orderReversed true for descending order.
	 */
	virtual void setFileShowMode(TeTypes::FileTypeFlags typeFlags, TeTypes::OrderType order, bool orderReversed) = 0;

signals:
	/** @brief Relays a command request to TeViewStore. */
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

	/**
	 * @brief Emitted when this view receives keyboard focus.
	 *
	 * Connected to TeViewStore::focusFolderViewChanged() to update the
	 * application's "current folder view" pointer.
	 */
	void focusIn();

protected:
	/**
	 * @brief Returns true when the event should be forwarded to the dispatcher.
	 *
	 * Navigation keys (arrows, Enter, etc.) are excluded; function keys and
	 * alphanumeric keys with optional Ctrl/Shift modifiers pass through.
	 */
	bool isDispatchable(TeTypes::WidgetType type, QEvent *event) const;

private:
	TeDispatchable* mp_dispatcher;
};
