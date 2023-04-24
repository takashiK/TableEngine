#pragma once

#include <QObject>
#include <QList>

class QWidget;

class TeCloseEventEmitter  : public QObject
{
	Q_OBJECT

public:
	TeCloseEventEmitter(QObject *parent=nullptr);
	~TeCloseEventEmitter();

	void oneShotRegister(QWidget* obj);

protected:
	virtual bool eventFilter(QObject* obj, QEvent* event);

signals:
	void closeEvent(QWidget* obj);

private:
	QList<const QWidget*> m_objList;
};
