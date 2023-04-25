#include "TeImageViewer.h"

#include "TeThumbnailProvider.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QListView>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QFileInfo>
#include <QImageReader>

#include <QFileIconProvider>

TeImageViewer::TeImageViewer(QWidget *parent)
	: QMainWindow(parent)
{
	mp_graphics = new QGraphicsView();
	mp_graphics->setScene(new QGraphicsScene());
	mp_image = new QGraphicsPixmapItem();
	mp_graphics->scene()->addItem(mp_image);
	mp_list = new QListView();
	mp_model = new QFileSystemModel();
	m_strechMode = StrechNone;
	m_sortColumn = 0;
	m_sortOrder = Qt::AscendingOrder;

	mp_list->setViewMode(QListView::IconMode);
	QAbstractFileIconProvider * provider = mp_model->iconProvider();
	mp_model->setIconProvider(new TeThumbnailProvider());

	setCentralWidget(mp_graphics);
	mp_dock = new QDockWidget("Image List", this);
	mp_list->setModel(mp_model);
	mp_dock->setWidget(mp_list);
	mp_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, mp_dock);
}

TeImageViewer::~TeImageViewer()
{
	delete mp_model;
}

bool TeImageViewer::open(const QString & path)
{
	QFileInfo info(path);
	if (info.isFile()) {
		mp_image->setPixmap(QPixmap(path));
		updateView();
		mp_model->setRootPath(info.absolutePath());
		mp_model->setFilter(QDir::Files);
		mp_model->setNameFilters(QStringList({ "*." + info.suffix() }));
		mp_model->setNameFilterDisables(false);
		mp_model->sort(m_sortColumn,m_sortOrder);
		mp_list->setRootIndex(mp_model->index(info.absolutePath()));
		mp_list->setCurrentIndex(mp_model->index(info.absoluteFilePath()));
		return true;
	}
	return false;
}

void TeImageViewer::nextImage()
{
	QModelIndex index = mp_model->index(mp_list->currentIndex().row() + 1, 0);
	if (index.isValid()) {
		mp_image->setPixmap(QPixmap(mp_model->filePath(index)));
		updateView();
		mp_list->setCurrentIndex(index);
	}
}

void TeImageViewer::prevImage()
{
	QModelIndex index = mp_model->index(mp_list->currentIndex().row() - 1, 0);
	if (index.isValid()) {
		mp_image->setPixmap(QPixmap(mp_model->filePath(index)));
		updateView();
		mp_list->setCurrentIndex(index);
	}
}

void TeImageViewer::showImageList()
{
	if (mp_dock->isHidden()) {
		mp_dock->show();
	}
}

void TeImageViewer::setStrechMode(Strech mode)
{
	if (mode != m_strechMode) {
		m_strechMode = mode;
		updateView();
	}
}

TeImageViewer::Strech TeImageViewer::strechMode() const
{
	return m_strechMode;
}

void TeImageViewer::setSortOrder(int column, Qt::SortOrder order)
{
	if (column != m_sortColumn || order != m_sortOrder) {
		m_sortColumn = column;
		m_sortOrder = order;
		mp_model->sort(column, order);
	}
}

QPair<int, Qt::SortOrder> TeImageViewer::sortOrder() const
{
	return QPair<int, Qt::SortOrder>(m_sortColumn,m_sortOrder);
}


void TeImageViewer::updateView()
{
}
