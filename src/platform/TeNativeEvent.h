#pragma once

#include <QObject>

/**
 * @file TeNativeEvent.h
 * @brief Declaration of TeNativeEvent.
 * @ingroup platform
 */


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
