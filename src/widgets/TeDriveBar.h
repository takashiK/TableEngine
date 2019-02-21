#pragma once

#include <QToolBar>

class QFileSystemWatcher;

class TeDriveBar : public QToolBar
{
	Q_OBJECT

public:
	TeDriveBar(QWidget *parent = Q_NULLPTR);
	TeDriveBar(const QString &title, QWidget *parent = Q_NULLPTR);
	virtual ~TeDriveBar();

signals:
	void changeDrive(const QString& path);

public slots:
	void driveChanged(const QString& path);

protected:
	void selectDrive(QAction* act, bool checked);

private:
	QAction* mp_current;
	QFileSystemWatcher* mp_watcher;
};
