#pragma once

#include <QMainwindow>

class QGraphicsView;
class QListView;
class QFileSystemModel;
class QDockWidget;
class QGraphicsPixmapItem;

class TeImageViewer  : public QMainWindow
{
	Q_OBJECT
public:
	enum Strech {
		StrechNone,
		StrechFit,
		StrechFill
	};

public:
	TeImageViewer(QWidget *parent= nullptr);
	virtual ~TeImageViewer();

	bool open(const QString& path);

	Strech strechMode() const;
	QPair<int, Qt::SortOrder> sortOrder() const;
	

public slots:
	void nextImage();
	void prevImage();
	void showImageList();
	void setStrechMode(Strech mode);
	void setSortOrder(int column, Qt::SortOrder order);

protected:
	void updateView();

private:
	QGraphicsView* mp_graphics;
	QGraphicsPixmapItem* mp_image;
	QListView* mp_list;
	QDockWidget* mp_dock;
	QFileSystemModel* mp_model;
	Strech m_strechMode;
	int m_sortColumn;
	Qt::SortOrder m_sortOrder;
};
