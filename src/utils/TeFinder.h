#pragma once

#include <QObject>
#include <QList>
#include <QMutex>
#include <QAtomicInt>
#include <QThread>

#include "TeFileInfo.h"
#include "TeSearchQuery.h"

class TeFolderView;

/*!
 * \brief Abstract base class for asynchronous file/archive searching.
 *
 * Subclasses implement doSearch() which runs on a dedicated worker thread.
 * Results accumulate in a mutex-protected list accessible from any thread via
 * results().  Each time a directory's worth of matching entries is ready,
 * itemsFound() is emitted in the GUI thread.
 */
class TeFinder : public QObject
{
	Q_OBJECT

public:
	explicit TeFinder(QObject* parent = nullptr);
	virtual ~TeFinder();

	virtual bool isValid() const = 0;

	/*! Returns true while the worker thread is running. */
	bool isRunning() const;

	void setRelatedView(TeFolderView* view);
	TeFolderView* relatedView() const;

	/*! Start an asynchronous search. Cancels and waits for any running search first. */
	void startSearch(const TeSearchQuery& query);

	/*! Request cancellation. Returns immediately; wait for searchCancelled() to confirm. */
	void cancelSearch();

	/*! Clear accumulated results. Safe to call only when no search is running. */
	void reset();

	/*! Thread-safe snapshot of all results collected so far. */
	QList<TeFileInfo> results() const;

	/*! Thread-safe count of all results collected so far. */
	int resultCount() const;

signals:
	/*! Emitted in the GUI thread for each directory batch of matching entries. */
	void itemsFound(const QList<TeFileInfo>& newItems);
	void searchFinished();
	void searchCancelled();

protected:
	/*!
	 * \brief Perform the actual search; called on the worker thread.
	 *
	 * Implementations must poll isCancelled() regularly and return early when
	 * true.  Call reportItems() to publish matching entries directory by directory.
	 */
	virtual void doSearch(const TeSearchQuery& query) = 0;

	/*! Returns true when cancelSearch() has been requested. Call from doSearch(). */
	bool isCancelled() const;

	/*!
	 * \brief Append items to the result list and emit itemsFound().
	 *
	 * Thread-safe; intended to be called from doSearch().
	 * itemsFound() is delivered to connected slots in the GUI thread.
	 */
	void reportItems(const QList<TeFileInfo>& items);

private slots:
	void onWorkerFinished();

private:
	mutable QMutex    m_resultsMutex;
	QList<TeFileInfo> m_results;
	QAtomicInt        m_cancelFlag;
	QThread*          m_workerThread = nullptr;
	TeFolderView*     mp_relatedView = nullptr;
};