#include "TeFindFolderView.h"

#include "TeEventFilter.h"
#include "TeFileTreeView.h"
#include "TeFileListView.h"
#include "utils/TeFinder.h"
#include "utils/TeFileInfo.h"

#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QFileIconProvider>
/**
 * @file TeFindFolderView.cpp
 * @brief Implementation of TeFindFolderView.
 * @ingroup widgets
 */

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------
namespace {

class TeFindStandardItem : public QStandardItem
{
public:
	TeFindStandardItem() : QStandardItem() {}
	explicit TeFindStandardItem(const QString& text) : QStandardItem(text) {}
	TeFindStandardItem(const QIcon& icon, const QString& text)
		: QStandardItem(icon, text) {}

	bool operator<(const QStandardItem& other) const override
	{
		int t = data(TeFileInfo::ROLE_TYPE).toInt();
		int o = other.data(TeFileInfo::ROLE_TYPE).toInt();
		if (t != o) return t > o;
		const int role = model() ? model()->sortRole() : Qt::DisplayRole;
		const QVariant l = data(role), r = other.data(role);
		if (l.userType() == QMetaType::LongLong)
			return l.toLongLong() < r.toLongLong();
		if (l.userType() == QMetaType::QDateTime)
			return l.toDateTime() < r.toDateTime();
		return l.toString().compare(r.toString(), Qt::CaseInsensitive) < 0;
	}
};

} // namespace

// ---------------------------------------------------------------------------
// Construction / destruction
// ---------------------------------------------------------------------------

TeFindFolderView::TeFindFolderView(QWidget* parent)
	: TeFolderView(parent)
{
	mp_treeView = new TeFileTreeView;
	mp_treeView->setFolderView(this);
	mp_listView = new TeFileListView;
	mp_listView->setFolderView(this);

	QSizePolicy treePolicy = mp_treeView->sizePolicy();
	treePolicy.setHorizontalStretch(1);
	mp_treeView->setSizePolicy(treePolicy);

	QSizePolicy listPolicy = mp_listView->sizePolicy();
	listPolicy.setHorizontalStretch(2);
	mp_listView->setSizePolicy(listPolicy);

	// Tree model: one flat row per search entry
	mp_treeView->setModel(new QStandardItemModel(this));
	mp_treeView->setHeaderHidden(true);
	mp_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	// List model: hit results for the active entry
	mp_listView->setModel(new QStandardItemModel(this));
	mp_listView->setViewMode(QListView::ListMode);
	mp_listView->setWrapping(true);
	mp_listView->setResizeMode(QListView::Adjust);
	mp_listView->setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
	mp_listView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
	        this, &TeFindFolderView::onTreeSelectionChanged);

	mp_treeEvent = new TeEventFilter(this);
	mp_treeEvent->setType(TeTypes::WT_TREEVIEW);
	mp_treeEvent->setDispatcher(this);
	mp_treeView->installEventFilter(mp_treeEvent);

	mp_listEvent = new TeEventFilter(this);
	mp_listEvent->setType(TeTypes::WT_LISTVIEW);
	mp_listEvent->setDispatcher(this);
	mp_listView->installEventFilter(mp_listEvent);

	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(mp_listView);
}

TeFindFolderView::~TeFindFolderView()
{
	// Cancel all running searches before destroying finders
	for (auto& entry : m_entries) {
		if (entry.finder) {
			entry.finder->cancelSearch();
			delete entry.finder;
		}
	}
	m_entries.clear();
}

// ---------------------------------------------------------------------------
// TeFolderView interface
// ---------------------------------------------------------------------------

TeTypes::WidgetType TeFindFolderView::getType() const
{
	return TeTypes::WT_FINDVIEW;
}

TeFileTreeView* TeFindFolderView::tree()
{
	return mp_treeView;
}

TeFileListView* TeFindFolderView::list()
{
	return mp_listView;
}

void TeFindFolderView::setRootPath(const QString& /*path*/)
{
	// Not applicable for find view
}

QString TeFindFolderView::rootPath()
{
	if (m_activeIndex < 0 || m_activeIndex >= m_entries.size()) return QString();
	return m_entries[m_activeIndex].query.targetPath;
}

void TeFindFolderView::setCurrentPath(const QString& path)
{
	// Select the tree entry whose targetPath matches
	for (int i = 0; i < m_entries.size(); ++i) {
		if (m_entries[i].query.targetPath == path) {
			const QModelIndex idx = treeModel()->index(i, 0);
			mp_treeView->setCurrentIndex(idx);
			return;
		}
	}
}

QString TeFindFolderView::currentPath()
{
	return rootPath();
}

void TeFindFolderView::moveNextPath()
{
	if (m_activeIndex + 1 < m_entries.size()) {
		const QModelIndex idx = treeModel()->index(m_activeIndex + 1, 0);
		mp_treeView->setCurrentIndex(idx);
	}
}

void TeFindFolderView::movePrevPath()
{
	if (m_activeIndex > 0) {
		const QModelIndex idx = treeModel()->index(m_activeIndex - 1, 0);
		mp_treeView->setCurrentIndex(idx);
	}
}

QStringList TeFindFolderView::getPathHistory() const
{
	QStringList list;
	for (const auto& entry : m_entries) {
		list.append(entry.query.targetPath);
	}
	return list;
}

TeFinder* TeFindFolderView::makeFinder()
{
	return nullptr;
}

void TeFindFolderView::setFileShowMode(TeTypes::FileTypeFlags /*typeFlags*/,
                                       TeTypes::OrderType      /*order*/,
                                       bool                    /*orderReversed*/)
{
	// Not applicable for find view
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

void TeFindFolderView::addSearchEntry(const TeSearchQuery& query, TeFinder* finder)
{
	// Add tree row entry
	const QString displayName = QFileInfo(query.targetPath).fileName();
	auto* item = new TeFindStandardItem(displayName.isEmpty() ? query.targetPath : displayName);
	item->setData(query.targetPath, TeFileInfo::ROLE_PATH);
	item->setEditable(false);
	treeModel()->appendRow(item);

	const int newIndex = m_entries.size();
	m_entries.append({query, finder});

	// Select the new entry (will trigger activateEntry via onTreeSelectionChanged)
	mp_treeView->setCurrentIndex(treeModel()->index(newIndex, 0));

	// Start the search after view is ready
	finder->startSearch(query);
}

// ---------------------------------------------------------------------------
// Private slots
// ---------------------------------------------------------------------------

void TeFindFolderView::onTreeSelectionChanged(const QModelIndex& current,
                                              const QModelIndex& /*previous*/)
{
	if (!current.isValid()) return;
	activateEntry(current.row());
}

void TeFindFolderView::onItemsFound(int offset, const QList<TeFileInfo>& newItems)
{
	// Guard: only handle signals from the currently active finder
	auto* senderFinder = qobject_cast<TeFinder*>(sender());
	if (m_activeIndex < 0 || m_activeIndex >= m_entries.size()) return;
	if (m_entries[m_activeIndex].finder != senderFinder) return;

	// Skip items that are already rendered from the initial snapshot
	const int skipCount = qMax(0, m_snapshotCount - offset);
	for (int i = skipCount; i < newItems.size(); ++i) {
		listModel()->appendRow(makeListRow(newItems[i]));
	}
}

void TeFindFolderView::onSearchFinished()
{
	// Update tree entry to reflect completed state (optional visual cue)
	auto* senderFinder = qobject_cast<TeFinder*>(sender());
	for (int i = 0; i < m_entries.size(); ++i) {
		if (m_entries[i].finder == senderFinder) {
			QStandardItem* item = treeModel()->item(i);
			if (item) {
				// Show result count in brackets
				const QString base = QFileInfo(m_entries[i].query.targetPath).fileName();
				item->setText(QString("%1 (%2)").arg(base.isEmpty()
					? m_entries[i].query.targetPath : base)
					.arg(senderFinder->resultCount()));
			}
			break;
		}
	}
}

void TeFindFolderView::onSearchCancelled()
{
	// Same as finished for visual purposes
	onSearchFinished();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void TeFindFolderView::activateEntry(int index)
{
	if (index < 0 || index >= m_entries.size()) return;

	// Disconnect previous active finder
	if (m_activeIndex >= 0 && m_activeIndex < m_entries.size()) {
		TeFinder* prev = m_entries[m_activeIndex].finder;
		if (prev) {
			disconnect(prev, &TeFinder::itemsFound,   this, &TeFindFolderView::onItemsFound);
			disconnect(prev, &TeFinder::searchFinished,   this, &TeFindFolderView::onSearchFinished);
			disconnect(prev, &TeFinder::searchCancelled,  this, &TeFindFolderView::onSearchCancelled);
		}
	}

	m_activeIndex = index;
	TeFinder* finder = m_entries[index].finder;

	// Clear the list model
	listModel()->clear();
	m_snapshotCount = 0;

	if (!finder) return;

	// 1. Connect BEFORE taking the snapshot to ensure no emitted batch is lost
	connect(finder, &TeFinder::itemsFound,
	        this,   &TeFindFolderView::onItemsFound,
	        Qt::QueuedConnection);
	connect(finder, &TeFinder::searchFinished,
	        this,   &TeFindFolderView::onSearchFinished,
	        Qt::QueuedConnection);
	connect(finder, &TeFinder::searchCancelled,
	        this,   &TeFindFolderView::onSearchCancelled,
	        Qt::QueuedConnection);

	// 2. Atomically take a snapshot — any batch emitted before this point is
	//    already queued and will arrive with offset < snapshotCount, so
	//    onItemsFound() will skip the overlapping portion automatically.
	int snapshotCount = 0;
	const QList<TeFileInfo> snapshot = finder->resultsSnapshot(snapshotCount);
	m_snapshotCount = snapshotCount;

	// 3. Populate list from snapshot
	for (const TeFileInfo& info : snapshot) {
		listModel()->appendRow(makeListRow(info));
	}
}

QList<QStandardItem*> TeFindFolderView::makeListRow(const TeFileInfo& info) const
{
	return info.exportItem(info.type == TeFileInfo::EN_FILE);
}

QStandardItemModel* TeFindFolderView::treeModel() const
{
	return qobject_cast<QStandardItemModel*>(mp_treeView->model());
}

QStandardItemModel* TeFindFolderView::listModel() const
{
	return qobject_cast<QStandardItemModel*>(mp_listView->model());
}
