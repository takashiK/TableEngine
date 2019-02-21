#pragma once

#include <QTreeWidget>

class TeTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	TeTreeWidget(QWidget *parent = nullptr);
	virtual ~TeTreeWidget();

	void setBuddy(QTreeWidget* p_buddy);

protected:
	virtual void dragEnterEvent(QDragEnterEvent* event);
	virtual void dropEvent(QDropEvent* event);
private:
	QTreeWidget* mp_buddy;
};
