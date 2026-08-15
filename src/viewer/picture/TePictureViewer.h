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

#pragma once

#include "TeEventEmitter.h"

#include <QMainWindow>
#include <QModelIndex>
#include <QThreadPool>

/**
 * @file TePictureViewer.h
 * @brief Declaration of TePictureViewer.
 * @ingroup viewer
 */


class QGraphicsView;
class QGraphicsPixmapItem;
class QListView;
class QFileSystemModel;
class TePictureThumbnailProxyModel;
class TePicturePrimaryLoadTask;
class QImage;

class TePictureViewer  : public QMainWindow
{
	Q_OBJECT

public:
	enum Stretch {
		StretchNone,
		StretchFit,
		StretchFill
	};

public:
	TePictureViewer(QWidget *parent=0);
	~TePictureViewer();

	bool open(const QString& path);

	Stretch stretchMode() const;
	std::pair<int, Qt::SortOrder> sortOrder() const;

signals:
	void stretchChanged(Stretch mode);

public slots:
	void nextImage();
	void prevImage();
	void showImageList(bool flag);
	void setStretchMode(Stretch mode);
	void setSortOrder(int column, Qt::SortOrder order);
	void rotateLeft();
	void rotateRight();

protected slots:
	void updateView(const QModelIndex& index = QModelIndex());

protected:
	void setupMenu();
	void loadImage(const QModelIndex& index);
	void primaryImageLoaded(quint64 generation, const QImage& image);

private:
	friend class TePicturePrimaryLoadTask;

	QGraphicsView* mp_graphics;
	QGraphicsPixmapItem* mp_image;
	QListView* mp_list;
	QDockWidget* mp_dock;
	QFileSystemModel* mp_model;
	TePictureThumbnailProxyModel* mp_thumbnailModel;
	Stretch m_stretchMode = StretchFit;
	int m_sortColumn = 0;
	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;
	QModelIndex m_imageIndex;
	TeEventEmitter* mp_emitter;
	int m_orientation = 1;
	int m_rotation = 0;
	quint64 m_loadGeneration = 0;
	QThreadPool m_primaryImageThreadPool;
};
