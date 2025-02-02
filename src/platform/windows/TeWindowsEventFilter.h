#pragma once

#include <QAbstractNativeEventFilter>

class QApplication;

class TeNativeEvent;

class TeWindowsEventFilter  : public QAbstractNativeEventFilter
{

public:
	TeWindowsEventFilter();
	~TeWindowsEventFilter();

	virtual bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result);

	void setNativeEvent(TeNativeEvent* p_event);

private:
	TeNativeEvent* mp_nativeEvent;
};
