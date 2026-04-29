#pragma once

#include <QList>
#include <QModelIndex>
#include "TeFolderView.h"
#include "utils/TeSearchQuery.h"
#include "utils/TeFileInfo.h"

/**
 * @file TeFindFolderView.h
 * @brief Two-pane folder view that displays asynchronous search results.
 * @ingroup widgets
 *
 * @see doc/markdown/widgets/TeFindFolderView.md
 */

class TeFileTreeView;
class TeFileListView;
class TeEventFilter;
class TeFinder;
class QStandardItem;
class QStandardItemModel;

/**
 * @class TeFindFolderView
 * @brief TeFolderView implementation that aggregates one or more search results.
 * @ingroup widgets
 *
 * @details Provides a virtual two-pane view where each tree node represents a
 * distinct saved search.  Search operations are executed asynchronously via a
 * TeFinder derived object; as results arrive through onItemsFound() they are
 * appended to the in-memory list model.
 *
 * Multiple simultaneous searches can be registered; the user switches between
 * them by selecting tree nodes.  addSearchEntry() transfers ownership of the
 * TeFinder to this view, which manages its lifetime.
 *
 * Unlike other TeFolderView subclasses, makeFinder() returns nullptr because
 * TeFindFolderView itself is the consumer of finder results rather than a
 * producer of a new finder.
 *
 * @see TeFolderView, TeFinder, TeSearchQuery, TeFileFinder, TeArchiveFinder
 * @see doc/markdown/widgets/TeFindFolderView.md
 */
class TeFindFolderView : public TeFolderView
{
	Q_OBJECT

public:
	explicit TeFindFolderView(QWidget* parent = nullptr);
	~TeFindFolderView();

	/** @brief Returns TeTypes::WT_FIND_FOLDER_VIEW. */
	TeTypes::WidgetType getType() const override;

	/** @brief Returns the tree-view pane. */
	TeFileTreeView* tree() override;
	/** @brief Returns the list-view pane. */
	TeFileListView* list() override;

	/** @brief Not meaningful for this view; always returns an empty string. */
	void    setRootPath(const QString& path) override;
	/** @brief Returns an empty string (no file-system root). */
	QString rootPath() override;

	/** @brief Not meaningful for this view; ignored. */
	void    setCurrentPath(const QString& path) override;
	/** @brief Returns the active search label string. */
	QString currentPath() override;

	/** @brief Navigates to the next registered search entry. */
	void        moveNextPath() override;
	/** @brief Navigates to the previous registered search entry. */
	void        movePrevPath() override;
	/** @brief Returns the list of registered search labels. */
	QStringList getPathHistory() const override;

	/**
	 * @brief Returns nullptr (TeFindFolderView does not produce a finder).
	 */
	TeFinder*   makeFinder() override;

	/**
	 * @brief Registers a new search and immediately starts it.
	 *
	 * Ownership of @p finder is transferred to this view.
	 * @param query  The search parameters.
	 * @param finder A fully configured, ready-to-start TeFinder.
	 */
	void addSearchEntry(const TeSearchQuery& query, TeFinder* finder);

public slots:
	/**
	 * @brief Updates file-type visibility flags and sort order.
	 * @param typeFlags     File type visibility flags.
	 * @param order         Sort column.
	 * @param orderReversed true for descending sort.
	 */
	void setFileShowMode(TeTypes::FileTypeFlags typeFlags,
	                     TeTypes::OrderType order,
	                     bool orderReversed) override;

private slots:
	/** @brief Handles tree-selection changes to switch the active search entry. */
	void onTreeSelectionChanged(const QModelIndex& current, const QModelIndex& previous);

	/**
	 * @brief Appends newly found items to the list model.
	 * @param offset   Start index in the finder's result list.
	 * @param newItems New results to append.
	 */
	void onItemsFound(int offset, const QList<TeFileInfo>& newItems);

	/** @brief Finalises the list model after a successful search. */
	void onSearchFinished();
	/** @brief Cleans up after a cancelled search. */
	void onSearchCancelled();

private:
	/** @brief Internal structure associating a query with its finder. */
	struct SearchEntry {
		TeSearchQuery query;         ///< The search parameters.
		TeFinder*     finder = nullptr; ///< The active finder (owned).
	};

	void activateEntry(int index);
	QList<QStandardItem*> makeListRow(const TeFileInfo& info) const;
	QStandardItemModel* treeModel() const;
	QStandardItemModel* listModel() const;

	TeFileTreeView* mp_treeView  = nullptr; ///< Left-pane tree widget.
	TeFileListView* mp_listView  = nullptr; ///< Right-pane list widget.
	TeEventFilter*  mp_treeEvent = nullptr; ///< Event filter for the tree pane.
	TeEventFilter*  mp_listEvent = nullptr; ///< Event filter for the list pane.

	QList<SearchEntry> m_entries;           ///< All registered search entries.
	int m_activeIndex    = -1;              ///< Index of the currently displayed entry.
	int m_snapshotCount  = 0;              ///< Number of results already rendered from snapshot.
};
