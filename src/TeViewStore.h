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

#include <QObject>
#include <QMainWindow>
#include "TeDispatcher.h"

/**
 * @file TeViewStore.h
 * @brief Central application store managing the main window and all folder views.
 * @ingroup main
 *
 * @details Declares TeViewStore, the top-level application object that owns the
 * UI widget tree, the TeDispatcher, and exposes properties for globally shared
 * view settings.
 *
 * @see doc/markdown/05_viewstore.md
 */

class TeMainWindow;
class TeFolderView;
class TeFileFolderView;
class TeArchiveFolderView;
class TeFindFolderView;
class TeEventEmitter;
class QTabWidget;
class TeDispatcher;
class QMainWindow;
class TeDriveBar;
class QSplitter;

/**
 * @class TeViewStore
 * @brief Top-level application store owning the widget tree and dispatcher.
 * @ingroup main
 *
 * @details TeViewStore orchestrates the entire UI lifecycle:
 * - Creates and shows TeMainWindow, the tab widgets, TeDriveBar, and the
 *   QSplitter layout via initialize().
 * - Manages a pool of TeFolderView instances distributed across two tab
 *   panels (TAB_LEFT / TAB_RIGHT).
 * - Exposes Qt properties for view-wide settings (selection mode, file sort
 *   order, view mode, etc.) that widgets bind to via signals.
 * - Tracks which TeFolderView currently has keyboard focus.
 *
 * @see doc/markdown/05_viewstore.md
 */
class TeViewStore : public QObject, public TeDispatchable
{
	Q_OBJECT

public:
	/**
	 * @brief Identifies the two tab panels available in the main window.
	 */
	enum TabPlace {
		TAB_LEFT  = 0, ///< Left tab panel.
		TAB_RIGHT = 1, ///< Right tab panel.
		TAB_MAX   = 2, ///< Number of tab panels.
	};

	Q_PROPERTY(TeTypes::SelectionMode selectionMode READ selectionMode WRITE setSelectionMode NOTIFY selectionModeChanged)
	Q_PROPERTY(bool driveBarVisible READ isDriveBarVisible WRITE setDriveBarVisible)
	Q_PROPERTY(bool statusBarVisible READ isStatusBarVisible WRITE setStatusBarVisible)
	Q_PROPERTY(bool toolBarVisible READ isToolBarVisible WRITE setToolBarVisible)
	Q_PROPERTY(bool navigationVisible READ isNavigationVisible WRITE setNavigationVisible)
	Q_PROPERTY(bool detailVisible READ isDetailVisible WRITE setDetailVisible)

	Q_PROPERTY(QFlags<TeTypes::FileInfo> fileInfoFlags READ fileInfoFlags WRITE setFileInfoFlags)
	Q_PROPERTY(QFlags<TeTypes::FileType> fileTypeFlags READ fileTypeFlags WRITE setFileTypeFlags)
	Q_PROPERTY(TeTypes::OrderType fileOrderBy READ fileOrderBy WRITE setFileOrderBy)
	Q_PROPERTY(bool fileOrderReversed READ isFileOrderReversed WRITE setFileOrderReversed)
	Q_PROPERTY(TeTypes::FileViewMode viewMode READ viewMode WRITE setViewMode)

	TeViewStore(QObject *parent = Q_NULLPTR);
	virtual  ~TeViewStore();

	/** @brief Returns TeTypes::WT_NONE (TeViewStore is not a widget). */
	virtual TeTypes::WidgetType getType() const;

	/**
	 * @brief Creates the main window, splitter, tabs, and drive bar.
	 *
	 * Must be called once before show().  Loads saved settings and connects
	 * all signals required for focus tracking and view state propagation.
	 */
	void initialize();

	/** @brief Shows the main window. */
	void show();

	/** @brief Saves current state and closes the main window. */
	void close();

	/** @brief (Re-)loads the menu structure from QSettings. */
	void loadMenu();

	/** @brief Loads general settings (window geometry, visibility flags) from QSettings. */
	void loadSetting();

	/** @brief Reloads key-binding settings into the TeDispatcher. */
	void loadKeySetting();

	/** @brief Restores the last-used folder paths for all open tabs. */
	void loadStatus();

	/**
	 * @brief Sets the dispatcher that processes widget events.
	 * @param p_dispatcher The application-wide TeDispatcher instance.
	 */
	void setDispatcher(TeDispatcher* p_dispatcher);

	/**
	 * @brief Forwards a widget event to the active folder view's dispatcher.
	 * @param type  The originating widget type.
	 * @param event The Qt event.
	 * @return true when the event was handled.
	 */
	virtual bool dispatch(TeTypes::WidgetType type, QEvent *event);
	virtual void execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

	/** @brief Returns the main window widget. */
	QWidget*  mainWindow();

	/** @brief Returns the tab panel index (TAB_LEFT/TAB_RIGHT) that has keyboard focus. */
	int currentTabPlace();

	/** @brief Returns the currently active tab index within currentTabPlace(). */
	int currentTabIndex();

	/**
	 * @brief Returns the tab panel index that contains @p view.
	 * @param view The folder view to locate.
	 * @return TAB_LEFT or TAB_RIGHT, or -1 if not found.
	 */
	int tabPlace(TeFolderView* view);

	/**
	 * @brief Returns the currently active TeFolderView in the given tab panel.
	 * @param place TAB_LEFT, TAB_RIGHT, or -1 for the focused panel.
	 */
	TeFolderView* getFolderView(int place = -1);

	/**
	 * @brief Returns all TeFolderView instances in the given panel.
	 * @param place TAB_LEFT, TAB_RIGHT, or -1 for all panels.
	 */
	QList<TeFolderView*> getFolderViews(int place = -1);

	/** @brief Returns the TeFolderView that currently has keyboard focus. */
	TeFolderView* currentFolderView();

	/**
	 * @brief Programmatically sets the focused folder view.
	 * @param view The view to focus.
	 */
	void setCurrentFolderView(TeFolderView* view);

	/**
	 * @brief Creates a new TeFileFolderView tab for @p path.
	 * @param path  The initial root path to display.
	 * @param place Target tab panel (-1 = active panel).
	 * @return The newly created view.
	 */
	TeFileFolderView* createFolderView( const QString& path, int place = -1 );

	/**
	 * @brief Creates a new TeArchiveFolderView tab for @p path.
	 * @param path  Path to the archive file.
	 * @param place Target tab panel (-1 = active panel).
	 * @return The newly created view.
	 */
	TeArchiveFolderView* createArchiveFolderView(const QString& path, int place = -1);

	/**
	 * @brief Returns the single TeFindFolderView, creating it on first call.
	 *
	 * If the view was previously removed from its tab it is re-added.
	 * @return The application-wide find view.
	 */
	TeFindFolderView* findFolderView();

	/**
	 * @brief Removes and destroys @p view from its tab panel.
	 * @param view The view to delete.
	 */
	void deleteFolderView( TeFolderView* view);

	/**
	 * @brief Moves @p view to @p position in @p place.
	 * @param view     The view to move.
	 * @param place    Destination tab panel (default: TAB_LEFT).
	 * @param position Target tab index (-1 = append).
	 */
	void moveFolderView( TeFolderView* view, int place = 0, int position = -1);

	/**
	 * @brief Navigates the active folder view to @p path.
	 * @param path    The new root/current path.
	 * @param newView If true, opens @p path in a new tab.
	 * @param place   Target tab panel (-1 = active).
	 */
	void changeRootPath(const QString& path, bool newView = false, int place = -1);

	/**
	 * @brief Registers @p widget as a floating window tracked by the store.
	 *
	 * The store monitors the close event and removes the widget from the
	 * floating list automatically.
	 * @param widget The floating widget to register.
	 */
	void registerFloatingWidget(QWidget* widget);

	/** @brief Returns the current item-selection mode. */
	TeTypes::SelectionMode selectionMode() const { return m_selectionMode; }
	/** @brief Returns true when the drive bar is visible. */
	bool isDriveBarVisible() const;
	/** @brief Returns true when the status bar is visible. */
	bool isStatusBarVisible() const;
	/** @brief Returns true when the toolbar is visible. */
	bool isToolBarVisible() const;
	/** @brief Returns true when the navigation pane is visible. */
	bool isNavigationVisible() const;
	/** @brief Returns true when the detail pane is visible. */
	bool isDetailVisible() const;

	/** @brief Returns the currently active file-information display flags. */
	TeTypes::FileInfoFlags fileInfoFlags() const { return m_fileInfoFlags; };
	/** @brief Returns the file-type visibility flags. */
	TeTypes::FileTypeFlags fileTypeFlags() const { return m_fileTypeFlags; };
	/** @brief Returns the current sort criterion. */
	TeTypes::OrderType fileOrderBy() const { return m_fileOrderBy; };
	/** @brief Returns true when the file list is sorted in descending order. */
	bool isFileOrderReversed() const { return m_fileOrderReversed; };
	/** @brief Returns the current list view display mode. */
	TeTypes::FileViewMode viewMode() const { return m_viewMode; };

signals:
	/** @brief Emitted when the selection mode changes. */
	void selectionModeChanged(TeTypes::SelectionMode mode);
	/** @brief Emitted when the file list view mode or info flags change. */
	void fileListViewModeChanged(TeTypes::FileInfoFlags infoFlags, TeTypes::FileViewMode viewMode);
	/** @brief Emitted when file type filters or sort order changes. */
	void fileListShowModeChanged(TeTypes::FileTypeFlags typeFlags, TeTypes::OrderType order, bool orderReversed);

public slots:
	/** @brief Called when a floating widget's close event fires. */
	void floatingWidgetClosed(QWidget* widget, QEvent* event);
	/** @brief Called when focus moves to a different folder view. */
	void focusFolderViewChanged(QWidget* widget, QEvent* event);
	/** @brief Sets the selection mode and notifies all folder views. */
	void setSelectionMode(TeTypes::SelectionMode mode);
	/** @brief Shows or hides the drive bar. */
	void setDriveBarVisible(bool visible);
	/** @brief Shows or hides the status bar. */
	void setStatusBarVisible(bool visible);
	/** @brief Shows or hides the toolbar. */
	void setToolBarVisible(bool visible);
	/** @brief Shows or hides the navigation pane. */
	void setNavigationVisible(bool visible);
	/** @brief Shows or hides the detail pane. */
	void setDetailVisible(bool visible);

	/** @brief Sets the file information display flags and notifies views. */
	void setFileInfoFlags(TeTypes::FileInfoFlags flags);
	/** @brief Sets the file-type visibility flags and notifies views. */
	void setFileTypeFlags(TeTypes::FileTypeFlags flags);
	/** @brief Sets the sort criterion and notifies views. */
	void setFileOrderBy(TeTypes::OrderType order);
	/** @brief Sets ascending/descending sort order and notifies views. */
	void setFileOrderReversed(bool reversed);
	/** @brief Sets the list display mode and notifies views. */
	void setViewMode(TeTypes::FileViewMode mode);

protected:
	/**
	 * @brief Inserts @p folderView into @p place's tab widget.
	 * @param folderView The view to add.
	 * @param place      Target tab panel (-1 = active panel).
	 */
	void addFolderView(TeFolderView* folderView, int place = -1);

signals:
	/** @brief Relays a command request from a folder view to the dispatcher. */
	void requestCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param);

private:
	TeMainWindow*        mp_mainWindow;
	QTabWidget*          mp_tab[TAB_MAX]; ///< Left and right tab panels.
	TeDriveBar*  mp_driveBar;
	QSplitter*     mp_split;
	TeTypes::SelectionMode m_selectionMode;

	int  m_currentTabPlace;
	bool m_isNavigationVisible;

	TeTypes::FileInfoFlags m_fileInfoFlags;
	TeTypes::FileTypeFlags m_fileTypeFlags;
	TeTypes::OrderType m_fileOrderBy;
	bool m_fileOrderReversed;
	TeTypes::FileViewMode m_viewMode;

	TeDispatcher* mp_dispatcher;
	TeEventEmitter* mp_closeEventEmitter;
	TeEventEmitter* mp_focusEventEmitter;
	QList<QWidget*> m_floatingWidgets;
	TeFindFolderView* mp_findView = nullptr;
};
