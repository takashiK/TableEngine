#include "TeFinder.h"
#include "widgets/TeFolderView.h"

#include <QCoreApplication>

TeFinder::TeFinder(QObject* parent)
	: QObject(parent)
{}

TeFinder::~TeFinder()
{
	m_cancelFlag.storeRelaxed(1);
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

TeFolderView* TeFinder::relatedView() const
{
	return mp_relatedView;
}

void TeFinder::startSearch(const TeSearchQuery& query)
{
	if (m_workerThread) {
		m_cancelFlag.storeRelaxed(1);
		m_workerThread->wait();
		delete m_workerThread;
		m_workerThread = nullptr;
	}
	reset();
	m_cancelFlag.storeRelaxed(0);

	m_workerThread = QThread::create([this, query]() {
		doSearch(query);
	});
	connect(m_workerThread, &QThread::finished,
	        this,            &TeFinder::onWorkerFinished,
	        Qt::QueuedConnection);
	m_workerThread->start();
}

void TeFinder::cancelSearch()
{
	m_cancelFlag.storeRelaxed(1);
}

void TeFinder::reset()
{
	QMutexLocker lock(&m_resultsMutex);
	m_results.clear();
}

QList<TeFileInfo> TeFinder::results() const
{
	QMutexLocker lock(&m_resultsMutex);
	return m_results;
}

int TeFinder::resultCount() const
{
	QMutexLocker lock(&m_resultsMutex);
	return m_results.count();
}

bool TeFinder::isCancelled() const
{
	return m_cancelFlag.loadRelaxed() != 0;
}

void TeFinder::reportItems(const QList<TeFileInfo>& items)
{
	{
		QMutexLocker lock(&m_resultsMutex);
		m_results.append(items);
	}
	emit itemsFound(items);
}

void TeFinder::onWorkerFinished()
{
	delete m_workerThread;
	m_workerThread = nullptr;

	if (m_cancelFlag.loadRelaxed() != 0) {
		emit searchCancelled();
	} else {
		emit searchFinished();
	}
}
