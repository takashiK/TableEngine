#pragma once

#include "TeEventEmitter.h"

#include <QMainWindow>
#include <QModelIndex>

class QGraphicsView;
class QGraphicsPixmapItem;
class QListView;
class QFileSystemModel;

class TePictureViewer  : public QMainWindow
{
	Q_OBJECT

public:
	enum Strech {
		StrechNone,
		StrechFit,
		StrechFill
	};

public:
	TePictureViewer(QWidget *parent=0);
	~TePictureViewer();

	bool open(const QString& path);

	Strech strechMode() const;
	QPair<int, Qt::SortOrder> sortOrder() const;

signals:
	void strechChanged(Strech mode);

public slots:
	void nextImage();
	void prevImage();
	void showImageList(bool flag);
	void setStrechMode(Strech mode);
	void setSortOrder(int column, Qt::SortOrder order);

protected slots:
	void updateView(const QModelIndex& index = QModelIndex());

protected:
	void setupMenu();

private:
	QGraphicsView* mp_graphics;
	QGraphicsPixmapItem* mp_image;
	QListView* mp_list;
	QDockWidget* mp_dock;
	QFileSystemModel* mp_model;
	Strech m_strechMode;
	int m_sortColumn;
	Qt::SortOrder m_sortOrder;
	QModelIndex m_imageIndex;
	TeEventEmitter* mp_emitter;
};
