#include "TePathFolderView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileListView.h"
#include "TeEventFilter.h"
#include "TeFileInfoAcsr.h"

#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QRegularExpression>

TePathFolderView::TePathFolderView(QWidget *parent)
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


	mp_treeView->setModel(new QStandardItemModel);

	mp_treeView->setHeaderHidden(true);
	mp_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

	mp_listView->setModel(new QStandardItemModel);
	mp_listView->setViewMode(QListView::ListMode);
	mp_listView->setWrapping(true);
	mp_listView->setResizeMode(QListView::Adjust);
	mp_listView->setSelectionMode(TeTypes::SELECTION_TABLE_ENGINE);
	mp_listView->setContextMenuPolicy(Qt::CustomContextMenu);

	//Initialize as write mode.
	clear();

	//Synchronize views when current directory changed.
	connect(mp_treeView->selectionModel(), &QItemSelectionModel::currentChanged,
		[this](const QModelIndex& current, const QModelIndex&/*previous*/)
		{ changeListView(current); });

	connect(mp_listView, &QListView::activated, this, &TePathFolderView::itemActivated);

	//connect to custom context menu.
	connect(mp_treeView, &QTreeView::customContextMenuRequested,
		[this](const QPoint& pos)
		{ showContextMenu(mp_treeView, pos); });

	connect(mp_listView, &QListView::customContextMenuRequested,
		[this](const QPoint& pos)
		{ showContextMenu(mp_listView, pos); });

	mp_treeEvent = new TeEventFilter();
	mp_treeEvent->setType(TeTypes::WT_TREEVIEW);
	mp_treeEvent->setDispatcher(this);
	mp_treeView->installEventFilter(mp_treeEvent);

	mp_listEvent = new TeEventFilter();
	mp_listEvent->setType(TeTypes::WT_LISTVIEW);
	mp_listEvent->setDispatcher(this);
	mp_listView->installEventFilter(mp_listEvent);


	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);
	layout->addWidget(mp_listView);
}

TePathFolderView::~TePathFolderView()
{}

TeFileTreeView* TePathFolderView::tree()
{
	return mp_treeView;
}

TeFileListView* TePathFolderView::list()
{
	return mp_listView;
}

void TePathFolderView::setRootPath(const QString& )
{
	//nothing to do.
}

QString TePathFolderView::rootPath()
{
	return QString("find:/");
}

void TePathFolderView::setCurrentPath(const QString& path)
{
	//only accept find://"Number"  or  "Number"
	QRegularExpression re("^(find://|\\./|)(\\d+)$");
	QRegularExpressionMatch match = re.match(path);
	if (match.hasMatch())
	{
		int number = match.captured(2).toInt();
		QStandardItemModel* model = dynamic_cast<QStandardItemModel*>(mp_treeView->model());
		if (number >= 0 && number < model->rowCount()) {
			QModelIndex index = model->index(number, 0);
			//setup listView by current index of treeView.


			m_history.push({rootPath(), currentPath()});
		}
	}
}

QString TePathFolderView::currentPath()
{
	QModelIndex index = mp_treeView->selectionModel()->currentIndex();
	if (index.isValid())
	{
		return rootPath() + QString::number(index.row());
	}
	return rootPath();
}

void TePathFolderView::moveNextPath()
{
	auto pair = m_history.next();
	if (pair != TeHistory::PathPair()) {
		setCurrentPath(pair.second);
	}
}

void TePathFolderView::movePrevPath()
{
	auto pair = m_history.previous();
	if (pair != TeHistory::PathPair()) {
		setCurrentPath(pair.second);
	}
}

QStringList TePathFolderView::getPathHistory() const
{
	//this folderview is not support path history. because it is not real file system.
	return QStringList();
}

void TePathFolderView::clear()
{
}

QStandardItem* TePathFolderView::createItem(const QIcon& icon, const QString& title, TeFileInfoAcsr* p_acsr)
{
	QStandardItem* treeItem = new QStandardItem(icon, title);
	qobject_cast<QStandardItemModel*>(tree()->model())->item(0)->appendRow(treeItem);

	QStandardItem* listItem = new QStandardItem(icon, title);
	qobject_cast<QStandardItemModel*>(list()->model())->item(0)->appendRow(listItem);

	treeItem->setData(QVariant::fromValue(listItem),ROLE_EXCHANGE);
	listItem->setData(QVariant::fromValue(treeItem),ROLE_EXCHANGE);



	return listItem;
}

/**
* @brief Change list view when tree view current index changed.
* @param index The current index of tree view.
 */
void TePathFolderView::changeListView(const QModelIndex& index)
{

}

void TePathFolderView::showContextMenu(const QAbstractItemView* p_view, const QPoint& pos)
{
}

void TePathFolderView::itemActivated(const QModelIndex& index)
{
}