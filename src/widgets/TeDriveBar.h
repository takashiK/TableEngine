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

#include <QToolBar>
#include <QChar>
#include <QMap>

class QFileSystemWatcher;

class TeDriveBar : public QToolBar
{
	Q_OBJECT

public:
	TeDriveBar(QWidget *parent = Q_NULLPTR);
	TeDriveBar(const QString &title, QWidget *parent = Q_NULLPTR);
	virtual ~TeDriveBar();

	void updateKey(const QChar& c);
	void loadQuickAccesses();
	void storeQuickAccesses();

	void addQuickAccess(const QString& path);
	void removeQuickAccess(const QString& path);
	void setQuickAccess(const QStringList& paths);
	QStringList quickAccess() const;
	void clear();

signals:
	void driveSelected(const QString& path);

public slots:
	void updateDrive(bool state);

protected:
	void selectDrive(const QString& path);

private:
	QChar m_key;
	QAction* mp_driveStart;
	QAction* mp_quickStart;
	QList<QAction*> m_driveActions;
	QList<QAction*> m_quickActions;
};
