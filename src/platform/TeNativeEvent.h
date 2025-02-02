#pragma once

#include <QObject>

class TeNativeEvent  : public QObject
{
	Q_OBJECT

public:
	TeNativeEvent(QObject *parent = nullptr);
	~TeNativeEvent();

	void changeMountState(bool state);

signals:
	void mountStateChanged(bool state);
};
