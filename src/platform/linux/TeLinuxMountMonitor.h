#pragma once

#include <QByteArray>
#include <QList>
#include <QObject>
#include <QTimer>

#include "platform/platform_util.h"

class QSocketNotifier;
class TeNativeEvent;

class TeLinuxMountMonitor : public QObject
{
public:
	explicit TeLinuxMountMonitor(TeNativeEvent* nativeEvent, QObject* parent = nullptr);
	~TeLinuxMountMonitor() override;

	void start();

private:
	void handleMountInfoNotification();
	void scheduleProjectionCheck();
	void checkProjection();
	QByteArray readMountInfo() const;

	TeNativeEvent* m_nativeEvent = nullptr;
	QList<TeDriveAction> m_snapshot;
	QByteArray m_mountInfo;
	QSocketNotifier* m_mountInfoNotifier = nullptr;
	QTimer m_debounceTimer;
	int m_mountInfoFd = -1;
};