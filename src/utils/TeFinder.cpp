#include "TeFinder.h"
#include "widgets/TeFolderView.h"

#include <QCoreApplication>

/**
 * @file TeFinder.cpp
 * @brief Implementation of TeFinder.
 * @ingroup utility
 */

TeFinder::TeFinder(QObject* parent)
	: QObject(parent)
{}

TeFinder::~TeFinder()
{
	m_cancelFlag.store(1, std::memory_order_relaxed);
	if (m_workerThread) {
		m_workerThread->wait();
		delete m_workerThread;
		m_workerThread = nullptr;
	}
	// Remove any queued onWorkerFinished calls that haven't been delivered yet.
	QCoreApplication::removePostedEvents(this);
}

bool TeFinder::isRunning() const
{
	return m_workerThread && m_workerThread->isRunning();
}

void TeFinder::setRelatedView(TeFolderView* view)
{
	mp_relatedView = view;
}

TeFolderView* TeFinder::relatedView() const noexcept
{
	return mp_relatedView;
}

void TeFinder::startSearch(const TeSearchQuery& query)
{
	if (!isValid()) {
		qWarning("TeFinder::startSearch: called on an invalid finder; ignoring");
		return;
	}
	reset();   // stops any running worker and clears results

	m_workerThread = QThread::create([this, query]() {
		doSearch(query);
	});
	connect(m_workerThread, &QThread::finished,
	        this,            &TeFinder::onWorkerFinished,
	        Qt::QueuedConnection);
	m_workerThread->start();
}

void TeFinder::cancelSearch() noexcept
{
	m_cancelFlag.store(1, std::memory_order_relaxed);
}

void TeFinder::reset()
{
	if (m_workerThread) {
		m_cancelFlag.store(1, std::memory_order_relaxed);
		m_workerThread->wait();
		delete m_workerThread;
		m_workerThread = nullptr;
		// Remove any queued onWorkerFinished calls so they don't fire on the
		// new thread that startSearch() is about to create.
		QCoreApplication::removePostedEvents(this, QEvent::MetaCall);
	}
	m_cancelFlag.store(0, std::memory_order_relaxed);
	QMutexLocker lock(&m_resultsMutex);
	m_results.clear();
}

QList<TeFileInfo> TeFinder::results() const
{
	QMutexLocker lock(&m_resultsMutex);
	return m_results;
}

QList<TeFileInfo> TeFinder::resultsSnapshot(int& count) const
{
	// COW: the copy under the lock is O(1) (ref-count only).
	// The first reportItems() call after this will detach m_results (O(n)),
	// but that cost is one-time per activateEntry() and acceptable.
	QMutexLocker lock(&m_resultsMutex);
	count = m_results.size();
	return m_results;
}

int TeFinder::resultCount() const
{
	QMutexLocker lock(&m_resultsMutex);
	return m_results.size();
}

bool TeFinder::isCancelled() const noexcept
{
	return m_cancelFlag.load(std::memory_order_relaxed) != 0;
}

void TeFinder::reportItems(const QList<TeFileInfo>& items)
{
	if (items.isEmpty())
		return;

	int startOffset;
	{
		QMutexLocker lock(&m_resultsMutex);
		startOffset = m_results.size();
		m_results.append(items);
	}
	emit itemsFound(startOffset, items);
}

void TeFinder::onWorkerFinished()
{
	delete m_workerThread;
	m_workerThread = nullptr;

	if (m_cancelFlag.load(std::memory_order_relaxed) != 0) {
		emit searchCancelled();
	} else {
		emit searchFinished();
	}
}
