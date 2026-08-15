/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#include "TePictureViewer.h"
#include "TePictureThumbnailDelegate.h"
#include "TePictureThumbnailProxyModel.h"
#include "utils/TeEmbeddedImageLoader.h"
#include "utils/TeQImageExifReader.h"
#include "TePictureSettings.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include <QListView>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QMetaObject>
#include <QRunnable>
#include <QThreadPool>
#include <QTransform>
#include <QSettings>

/**
 * @file TePictureViewer.cpp
 * @brief Declaration of TePictureViewer.
 * @ingroup viewer
 */

namespace {
/**
 * @brief Builds a view transform that corrects an image to upright based on its
 *        EXIF orientation.
 * @param orientation EXIF orientation value (1-8).
 * @return Transform to apply to the QGraphicsView (rotation and/or mirroring).
 */
QTransform exifViewTransform(int orientation)
{
	switch (orientation) {
		case 2: return QTransform().scale(-1, 1);
		case 3: return QTransform().rotate(180);
		case 4: return QTransform().scale(1, -1);
		case 5: return QTransform().rotate(90) * QTransform().scale(-1, 1);
		case 6: return QTransform().rotate(90);
		case 7: return QTransform().rotate(90) * QTransform().scale(1, -1);
		case 8: return QTransform().rotate(270);
		default: return QTransform();
	}
}
} // namespace

class TePicturePrimaryLoadTask : public QRunnable
{
public:
	TePicturePrimaryLoadTask(const QString& path, int orientation, quint64 generation,
		TePictureViewer* viewer)
		: m_path(path)
		, m_orientation(orientation)
		, m_generation(generation)
		, mp_viewer(viewer)
	{
	}

	void run() override
	{
		TeEmbeddedImageSelection selection;
		selection.primaryOrientation = m_orientation;
		const QImage image = teDecodeEmbeddedImage(m_path, TeEmbeddedImageSet{}, selection,
			QSize(), m_orientation);
		QMetaObject::invokeMethod(mp_viewer, [viewer = mp_viewer, generation = m_generation, image]() {
			viewer->primaryImageLoaded(generation, image);
		}, Qt::QueuedConnection);
	}

private:
	QString m_path;
	int m_orientation;
	quint64 m_generation;
	TePictureViewer* mp_viewer;
};

TePictureViewer::TePictureViewer(QWidget *parent)
	: QMainWindow(parent)
{
	mp_graphics = new QGraphicsView();
	mp_graphics->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
	mp_graphics->setScene(new QGraphicsScene());
	mp_image = new QGraphicsPixmapItem();
	mp_graphics->scene()->addItem(mp_image);
	mp_list = new QListView();
	mp_model = new QFileSystemModel();
	mp_thumbnailModel = new TePictureThumbnailProxyModel(this);
	mp_thumbnailModel->setThumbnailLoadingEnabled(false);
	mp_thumbnailModel->setSourceModel(mp_model);

	mp_list->setIconSize(QSize(96, 96));
	mp_list->setUniformItemSizes(true);
	mp_list->setLayoutMode(QListView::SinglePass);
	mp_list->setItemDelegate(new TePictureThumbnailDelegate(mp_list));
	mp_list->setModel(mp_thumbnailModel);

	connect(mp_list->selectionModel(),&QItemSelectionModel::currentChanged, this, &TePictureViewer::updateView);

	mp_emitter = new TeEventEmitter();
	mp_emitter->addEmitter(mp_graphics->viewport());
	mp_emitter->addEventType(QEvent::Resize);
	mp_emitter->addEventType(QEvent::LayoutRequest);
	connect(mp_emitter, &TeEventEmitter::emitEvent, [this](QWidget* , QEvent* ) {updateView();});

	setCentralWidget(mp_graphics);

	mp_dock = new QDockWidget("Image List");
	mp_dock->setWidget(mp_list);
	mp_dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	connect(mp_dock, &QDockWidget::visibilityChanged, mp_thumbnailModel,
		&TePictureThumbnailProxyModel::setThumbnailLoadingEnabled);
	addDockWidget(Qt::LeftDockWidgetArea, mp_dock);
	mp_dock->close();

	setupMenu();
	m_primaryImageThreadPool.setMaxThreadCount(1);

	QSettings settings;
	resize(settings.value(SETTING_PIC_VIEWER_WINDOW_WIDTH, 800).toInt(),
	       settings.value(SETTING_PIC_VIEWER_WINDOW_HEIGHT, 600).toInt());
}

TePictureViewer::~TePictureViewer()
{
	++m_loadGeneration;

	QSettings settings;
	settings.setValue(SETTING_PIC_VIEWER_WINDOW_WIDTH, width());
	settings.setValue(SETTING_PIC_VIEWER_WINDOW_HEIGHT, height());
	
	m_primaryImageThreadPool.clear();
	m_primaryImageThreadPool.waitForDone();
	delete mp_thumbnailModel;
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
		mp_thumbnailModel->clearThumbnailCache();
		mp_list->setRootIndex(mp_thumbnailModel->mapFromSource(mp_model->index(info.absolutePath())));

		mp_list->setCurrentIndex(mp_thumbnailModel->mapFromSource(mp_model->index(info.absoluteFilePath())));
		return true;
	}
	return false;
}

TePictureViewer::Stretch TePictureViewer::stretchMode() const
{
	return m_stretchMode;
}

std::pair<int, Qt::SortOrder> TePictureViewer::sortOrder() const
{
	return std::pair<int, Qt::SortOrder>{m_sortColumn, m_sortOrder};
}

void TePictureViewer::nextImage()
{
	QModelIndex index = mp_thumbnailModel->index(mp_list->currentIndex().row() + 1, 0, mp_list->currentIndex().parent());
	if (index.isValid()) {
		mp_list->setCurrentIndex(index);
	}
}

void TePictureViewer::prevImage()
{
	QModelIndex index = mp_thumbnailModel->index(mp_list->currentIndex().row() - 1, 0, mp_list->currentIndex().parent());
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

void TePictureViewer::setStretchMode(Stretch mode)
{
	if (mode != m_stretchMode) {
		m_stretchMode = mode;
		updateView();
		emit stretchChanged(mode);
	}
}

void TePictureViewer::setSortOrder(int column, Qt::SortOrder order)
{
	if (column != m_sortColumn || order != m_sortOrder) {
		m_sortColumn = column;
		m_sortOrder = order;
		mp_thumbnailModel->sort(column, order);
	}
}

void TePictureViewer::updateView(const QModelIndex& index)
{
	if (index.isValid() && (m_imageIndex != index)) {
		loadImage(index);
		m_imageIndex = index;
	}

	mp_graphics->setSceneRect(mp_image->boundingRect());
	mp_graphics->setTransform(exifViewTransform(m_orientation));
	mp_graphics->rotate(m_rotation);
	switch (m_stretchMode) {
		case StretchNone:
			break;
		case StretchFit:
			mp_graphics->fitInView(mp_image, Qt::KeepAspectRatio);
			break;
		case StretchFill:
			mp_graphics->fitInView(mp_image, Qt::IgnoreAspectRatio);
			break;

	}
}

void TePictureViewer::loadImage(const QModelIndex& index)
{
	const QString path = mp_model->filePath(mp_thumbnailModel->mapToSource(index));

	TeQImageExifReader reader;
	const QSize targetSize = mp_graphics->viewport()->size();
	const TeEmbeddedImageSet images = reader.scanImages(path);
	TeEmbeddedImageSelection selection = teSelectEmbeddedImage(images, targetSize);
	m_orientation = selection.primaryOrientation;
	if (images.images().isEmpty()) {
		m_orientation = reader.read(path).value(QStringLiteral("Orientation"), QStringLiteral("1")).toInt();
		if (m_orientation < 1 || m_orientation > 8)
			m_orientation = 1;
		selection.primaryOrientation = m_orientation;
	}

	if (selection.useEmbeddedImage && selection.image.byteSize > 0 &&
		selection.image.byteSize <= 4 * 1024 * 1024) {
		const QImage preview = teDecodeEmbeddedImage(path, images, selection, targetSize,
			m_orientation, false);
		mp_image->setPixmap(preview.isNull() ? QPixmap() : QPixmap::fromImage(preview));
	} else {
		mp_image->setPixmap(QPixmap());
	}

	m_rotation = 0;
	const quint64 generation = ++m_loadGeneration;
	m_primaryImageThreadPool.clear();
	auto* task = new TePicturePrimaryLoadTask(path, m_orientation, generation, this);
	task->setAutoDelete(true);
	m_primaryImageThreadPool.start(task);
}

void TePictureViewer::primaryImageLoaded(quint64 generation, const QImage& image)
{
	if (generation != m_loadGeneration || image.isNull())
		return;
	mp_image->setPixmap(QPixmap::fromImage(image));
	updateView();
}

void TePictureViewer::rotateRight()
{
	m_rotation = (m_rotation + 90) % 360;
	updateView();
}

void TePictureViewer::rotateLeft()
{
	m_rotation = (m_rotation + 270) % 360;
	updateView();
}

void TePictureViewer::setupMenu()
{
	QMenu* menu = nullptr;
	QAction* action = nullptr;

	menu = menuBar()->addMenu(tr("&File"));
	action = menu->addAction(tr("&Quit"));
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
	action = menu->addAction(tr("Rotate &Right"));
	action->setShortcuts({ QKeySequence(Qt::Key_BracketRight), QKeySequence(Qt::CTRL | Qt::Key_R) });
	connect(action, &QAction::triggered, this, &TePictureViewer::rotateRight);
	action = menu->addAction(tr("Rotate &Left"));
	action->setShortcuts({ QKeySequence(Qt::Key_BracketLeft), QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_R) });
	connect(action, &QAction::triggered, this, &TePictureViewer::rotateLeft);
	menu->addSeparator();
	QMenu* subMenu = menu->addMenu(tr("Stretch Mode"));
	action = subMenu->addAction(tr("None"));
	action->setCheckable(true);
	action->setChecked(stretchMode() == TePictureViewer::StretchNone);
	connect(this, &TePictureViewer::stretchChanged, [action](TePictureViewer::Stretch mode) {action->setChecked(mode == TePictureViewer::StretchNone); });
	connect(action, &QAction::triggered, this, [this]() {setStretchMode(StretchNone); });
	action = subMenu->addAction(tr("Fit"));
	action->setCheckable(true);
	action->setChecked(stretchMode() == TePictureViewer::StretchFit);
	connect(this, &TePictureViewer::stretchChanged, [action](TePictureViewer::Stretch mode) {action->setChecked(mode == TePictureViewer::StretchFit); });
	connect(action, &QAction::triggered, this, [this]() {setStretchMode(StretchFit); });
	action = subMenu->addAction(tr("Fill"));
	action->setCheckable(true);
	action->setChecked(stretchMode() == TePictureViewer::StretchFill);
	connect(this, &TePictureViewer::stretchChanged, [action](TePictureViewer::Stretch mode) {action->setChecked(mode == TePictureViewer::StretchFill); });
	connect(action, &QAction::triggered, this, [this]() {setStretchMode(StretchFill); });
}
