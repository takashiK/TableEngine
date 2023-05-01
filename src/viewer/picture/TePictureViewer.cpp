#include "TePictureViewer.h"
#include "widgets/TeThumbnailProvider.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QListView>
#include <QDockWidget>
#include <QFileSystemModel>

TePictureViewer::TePictureViewer(QWidget *parent)
	: QMainWindow(parent)
{
	mp_graphics = new QGraphicsView();
	mp_graphics->setScene(new QGraphicsScene());
	mp_image = new QGraphicsPixmapItem();
	mp_graphics->scene()->addItem(mp_image);
	mp_list = new QListView();
	mp_model = new QFileSystemModel();
	m_strechMode = StrechFit;
	m_sortColumn = 0;
	m_sortOrder = Qt::AscendingOrder;

	mp_list->setIconSize(QSize(96, 96));

	mp_list->setLayoutMode(QListView::Batched);
	mp_list->setModel(mp_model);

	connect(mp_list->selectionModel(),&QItemSelectionModel::currentChanged, this, &TePictureViewer::updateView);

	mp_model->setIconProvider(TeThumbnailProvider::iconProvider());

	
	mp_emitter = new TeEventEmitter();
	mp_emitter->addEmitter(mp_graphics->viewport());
	mp_emitter->addEventType(QEvent::Resize);
	mp_emitter->addEventType(QEvent::LayoutRequest);
	connect(mp_emitter, &TeEventEmitter::emitEvent, [this](QWidget* widget, QEvent* event) {updateView();});

	setCentralWidget(mp_graphics);

	mp_dock = new QDockWidget("Image List");
	mp_dock->setWidget(mp_list);
	mp_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, mp_dock);
	mp_dock->close();

	setupMenu();
}

TePictureViewer::~TePictureViewer()
{
	delete mp_model;
	delete mp_emitter;
}

bool TePictureViewer::open(const QString & path)
{
	QFileInfo info(path);
	if (info.isFile()) {
		mp_model->setRootPath(info.absolutePath());
		mp_model->setFilter(QDir::Files);
		mp_model->setNameFilters(QStringList({ "*." + info.suffix() }));
		mp_model->setNameFilterDisables(false);
		mp_model->sort(m_sortColumn, m_sortOrder);
		mp_list->setRootIndex(mp_model->index(info.absolutePath()));

		mp_list->setCurrentIndex(mp_model->index(info.absoluteFilePath()));
		return true;
	}
	return false;
}

TePictureViewer::Strech TePictureViewer::strechMode() const
{
	return m_strechMode;
}

QPair<int, Qt::SortOrder> TePictureViewer::sortOrder() const
{
	return QPair<int, Qt::SortOrder>(m_sortColumn, m_sortOrder);
}

void TePictureViewer::nextImage()
{
	QModelIndex index = mp_model->index(mp_list->currentIndex().row() + 1, 0, mp_list->currentIndex().parent());
	if (index.isValid()) {
		mp_list->setCurrentIndex(index);
	}
}

void TePictureViewer::prevImage()
{
	QModelIndex index = mp_model->index(mp_list->currentIndex().row() - 1, 0, mp_list->currentIndex().parent());
	if (index.isValid()) {
		mp_list->setCurrentIndex(index);
	}
}

void TePictureViewer::showImageList(bool flag)
{
	if (flag) {
		mp_dock->show();
	} 
	else {
		mp_dock->hide();
	}
}

void TePictureViewer::setStrechMode(Strech mode)
{
	if (mode != m_strechMode) {
		m_strechMode = mode;
		updateView();
		emit strechChanged(mode);
	}
}

void TePictureViewer::setSortOrder(int column, Qt::SortOrder order)
{
	if (column != m_sortColumn || order != m_sortOrder) {
		m_sortColumn = column;
		m_sortOrder = order;
		mp_model->sort(column, order);
	}
}

void TePictureViewer::updateView(const QModelIndex& index)
{
	if (index.isValid() && (m_imageIndex != index)) {
		mp_image->setPixmap(QPixmap(mp_model->filePath(index)));
		m_imageIndex = index;
	}

	switch (m_strechMode) {
		case StrechNone:
			mp_graphics->setSceneRect(mp_image->boundingRect());
			mp_graphics->resetTransform();
			break;
		case StrechFit:
			mp_graphics->setSceneRect(mp_image->boundingRect());
			mp_graphics->fitInView(mp_image, Qt::KeepAspectRatio);
			break;
		case StrechFill:
			mp_graphics->setSceneRect(mp_image->boundingRect());
			mp_graphics->fitInView(mp_image, Qt::IgnoreAspectRatio);
			break;

	}
}

void TePictureViewer::setupMenu()
{
	QMenu* menu = nullptr;
	QAction* action = nullptr;

	menu = menuBar()->addMenu(tr("&File"));
	action = menu->addAction(tr("&Exit"));
	action->setShortcuts({ QKeySequence(Qt::Key_Escape)});
	connect(action, &QAction::triggered, this, &TePictureViewer::close);

	menu = menuBar()->addMenu(tr("&View"));
	action = menu->addAction(tr("&Next Image"));
	action->setShortcuts({QKeySequence(Qt::Key_Space),QKeySequence(Qt::Key_Down)});
	connect(action, &QAction::triggered, this, &TePictureViewer::nextImage);
	action = menu->addAction(tr("&Previous Image"));
	action->setShortcuts({ QKeySequence(Qt::ShiftModifier | Qt::Key_Space ),QKeySequence(Qt::Key_Up) });
	connect(action, &QAction::triggered, this, &TePictureViewer::prevImage);
	action = menu->addAction(tr("&Show Image List"));
	connect(mp_dock, &QDockWidget::visibilityChanged, action, &QAction::setChecked);
	action->setCheckable(true);
	action->setChecked(mp_dock->isVisible());
	connect(action, &QAction::toggled, this, &TePictureViewer::showImageList);
	menu->addSeparator();
	QMenu* subMenu = menu->addMenu(tr("Strech Mode"));
	action = subMenu->addAction(tr("None"));
	action->setCheckable(true);
	action->setChecked(strechMode() == TePictureViewer::StrechNone);
	connect(this, &TePictureViewer::strechChanged, [action](TePictureViewer::Strech mode) {action->setChecked(mode == TePictureViewer::StrechNone); });
	connect(action, &QAction::triggered, this, [this]() {setStrechMode(StrechNone); });
	action = subMenu->addAction(tr("Fit"));
	action->setCheckable(true);
	action->setChecked(strechMode() == TePictureViewer::StrechFit);
	connect(this, &TePictureViewer::strechChanged, [action](TePictureViewer::Strech mode) {action->setChecked(mode == TePictureViewer::StrechFit); });
	connect(action, &QAction::triggered, this, [this]() {setStrechMode(StrechFit); });
	action = subMenu->addAction(tr("Fill"));
	action->setCheckable(true);
	action->setChecked(strechMode() == TePictureViewer::StrechFill);
	connect(this, &TePictureViewer::strechChanged, [action](TePictureViewer::Strech mode) {action->setChecked(mode == TePictureViewer::StrechFill); });
	connect(action, &QAction::triggered, this, [this]() {setStrechMode(StrechFill); });
}
