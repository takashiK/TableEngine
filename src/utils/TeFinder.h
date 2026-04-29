#pragma once

#include <QObject>
#include <QList>
#include <QMutex>
#include <QAtomicInt>
#include <QThread>

#include "TeFileInfo.h"
#include "TeSearchQuery.h"

/**
 * @file TeFinder.h
 * @brief Abstract base class for asynchronous file and archive search.
 * @ingroup utility
 *
 * @see doc/markdown/utils/TeFinder.md
 */

class TeFolderView;

/**
 * @brief Abstract base class for asynchronous file/archive searching.
 * @ingroup utility
 *
 * @details Subclasses implement doSearch() which runs on a dedicated worker
 * thread.  Results accumulate in a mutex-protected list accessible from any
 * thread via results().  Each time a directory batch of matching entries is
 * ready, itemsFound() is emitted in the GUI thread.
 *
 * @see TeFileFinder, TeArchiveFinder, TeSearchQuery
 * @see doc/markdown/utils/TeFinder.md
 */
class TeFinder : public QObject
{
	Q_OBJECT

public:
	explicit TeFinder(QObject* parent = nullptr);
	virtual ~TeFinder();

	/** @brief Returns true when the search source is usable. */
	virtual bool isValid() const = 0;

	/** @brief Returns true while the worker thread is running. */
	bool isRunning() const;

	/**
	 * @brief Sets the folder view that initiated the search.
	 * @param view The folder view to associate with this finder.
	 */
	void setRelatedView(TeFolderView* view);
	/** @brief Returns the associated folder view. */
	TeFolderView* relatedView() const;

	/**
	 * @brief Starts an asynchronous search.
	 *
	 * Cancels and waits for any running search before starting a new one.
	 * @param query The search parameters.
	 */
	void startSearch(const TeSearchQuery& query);

	/**
	 * @brief Requests cancellation.
	 *
	 * Returns immediately; wait for searchCancelled() to confirm.
	 */
	void cancelSearch();

	/** @brief Clears accumulated results.  Safe to call only when no search is running. */
	void reset();

	/** @brief Thread-safe snapshot of all results collected so far. */
	QList<TeFileInfo> results() const;

	/** @brief Thread-safe count of all results collected so far. */
	int resultCount() const;

	/**
	 * @brief Atomically returns all current results and their count.
	 *
	 * Connect itemsFound() first, then call this to avoid the
	 * connect/snapshot race: any batch emitted between connect() and this call
	 * will have offset < count and can be safely skipped in the slot.
	 * @param count Receives the number of items in the returned list.
	 * @return Thread-safe copy of all results up to the snapshot point.
	 */
	QList<TeFileInfo> resultsSnapshot(int& count) const;

signals:
	/**
	 * @brief Emitted in the GUI thread for each directory batch of matching entries.
	 *
	 * @param offset  Index of the first item of this batch within results().
	 *                Use with resultsSnapshot() to avoid race-condition gaps.
	 * @param newItems The newly found items in this batch.
	 */
	void itemsFound(int offset, const QList<TeFileInfo>& newItems);

	/** @brief Emitted after doSearch() returns normally. */
	void searchFinished();
	/** @brief Emitted after doSearch() returns due to cancellation. */
	void searchCancelled();

protected:
	/**
	 * @brief Performs the actual search on the worker thread.
	 *
	 * Implementations must poll isCancelled() regularly and return early when
	 * true.  Call reportItems() to publish matching entries directory by directory.
	 * @param query The search parameters.
	 */
	virtual void doSearch(const TeSearchQuery& query) = 0;

	/** @brief Returns true when cancelSearch() has been requested. */
	bool isCancelled() const;

	/**
	 * @brief Appends items to the result list and emits itemsFound().
	 *
	 * Thread-safe; intended to be called from doSearch().
	 * itemsFound() is delivered to connected slots in the GUI thread.
	 * @param items The newly found items to publish.
	 */
	void reportItems(const QList<TeFileInfo>& items);

private slots:
	void onWorkerFinished();

private:
	mutable QMutex    m_resultsMutex;      ///< Guards m_results.
	QList<TeFileInfo> m_results;           ///< Accumulated search results.
	QAtomicInt        m_cancelFlag;        ///< Set to 1 when cancellation is requested.
	QThread*          m_workerThread = nullptr; ///< Worker thread running doSearch().
	TeFolderView*     mp_relatedView = nullptr; ///< Associated folder view.
};