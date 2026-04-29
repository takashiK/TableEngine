#pragma once

#include "widgets/TeFolderView.h"
#include "utils/TeHistory.h"
#include "utils/TeFileInfo.h"

/**
 * @file TePathFolderView.h
 * @brief Two-pane folder view displaying a user-defined virtual directory tree.
 * @ingroup widgets
 */

class TeEventFilter;
class QAbstractItemView;
class QStandardItem;
class TeFileInfoAcsr;

/**
 * @class TePathFolderView
 * @brief TeFolderView implementation backed by a QStandardItemModel of
 *        user-composed virtual paths.
 * @ingroup widgets
 *
 * @details Unlike TeFileFolderView, TePathFolderView does not reflect the
 * real file system directly.  Instead, callers populate its model by calling
 * createItem() to build a virtual tree of favourites or recent paths.
 * Navigation history is managed by a TeHistory instance.
 *
 * The internal Role enum reserves a custom model role (ROLE_EXCHANGE) for
 * attaching a TeFileInfoAcsr payload to items.
 *
 * @see TeFolderView, TeFileInfoAcsr, TeHistory
 */

class TePathFolderView : public TeFolderView
{
	Q_OBJECT

	enum Role {
		ROLE_EXCHANGE = TeFileInfo::ROLE_USER_START,
	};

public:
	TePathFolderView(QWidget *parent = nullptr);
	~TePathFolderView();

	virtual TeFileTreeView* tree();
	virtual TeFileListView* list();

	virtual void setRootPath(const QString& path);
	virtual QString rootPath();
	virtual void setCurrentPath(const QString& path);
	virtual QString currentPath();

	virtual void moveNextPath();
	virtual void movePrevPath();
	virtual QStringList getPathHistory() const;

	void clear();
	QStandardItem* createItem(const QIcon& icon, const QString& title, TeFileInfoAcsr* p_acsr=nullptr);

protected:
	void changeListView(const QModelIndex& index);
	void showContextMenu(const QAbstractItemView* p_view, const QPoint& pos);

protected slots:
	void itemActivated(const QModelIndex& index);

private:
	TeFileListView* mp_listView;
	TeFileTreeView* mp_treeView;

	TeEventFilter* mp_treeEvent;
	TeEventFilter* mp_listEvent;

	TeHistory m_history;
};
