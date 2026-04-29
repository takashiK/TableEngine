#pragma once

#include <QList>
#include <QModelIndex>
#include "TeFolderView.h"
#include "utils/TeSearchQuery.h"
#include "utils/TeFileInfo.h"

class TeFileTreeView;
class TeFileListView;
class TeEventFilter;
class TeFinder;
class QStandardItem;
class QStandardItemModel;

class TeFindFolderView : public TeFolderView
{
	Q_OBJECT

public:
	explicit TeFindFolderView(QWidget* parent = nullptr);
	~TeFindFolderView();

	// TeFolderView interface
	TeTypes::WidgetType getType() const override;

	TeFileTreeView* tree() override;
	TeFileListView* list() override;

	void    setRootPath(const QString& path) override;
	QString rootPath() override;
	void    setCurrentPath(const QString& path) override;
	QString currentPath() override;

	void        moveNextPath() override;
	void        movePrevPath() override;
	QStringList getPathHistory() const override;
	TeFinder*   makeFinder() override;

	/*!
	 * Register a new search entry and start the search immediately.
	 * Ownership of \a finder is transferred to this view.
	 */
	void addSearchEntry(const TeSearchQuery& query, TeFinder* finder);

public slots:
	void setFileShowMode(TeTypes::FileTypeFlags typeFlags,
	                     TeTypes::OrderType order,
	                     bool orderReversed) override;

private slots:
	void onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);
	void onItemsFound(int offset, const QList<TeFileInfo>& newItems);
	void onSearchFinished();
	void onSearchCancelled();

private:
	struct SearchEntry {
		TeSearchQuery query;
		TeFinder*     finder = nullptr;
	};

	void activateEntry(int index);
	QList<QStandardItem*> makeListRow(const TeFileInfo& info) const;
	QStandardItemModel* treeModel() const;
	QStandardItemModel* listModel() const;

	TeFileTreeView* mp_treeView  = nullptr;
	TeFileListView* mp_listView  = nullptr;
	TeEventFilter*  mp_treeEvent = nullptr;
	TeEventFilter*  mp_listEvent = nullptr;

	QList<SearchEntry> m_entries;
	int m_activeIndex    = -1;
	int m_snapshotCount  = 0;  //!< Number of results already rendered from snapshot
};
