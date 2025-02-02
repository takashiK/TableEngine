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

#include "TeDriveBar.h"
#include "platform/platform_util.h"

#include <QFileIconProvider>
#include <QDir>
#include <QStorageInfo>
#include <QApplication>
#include <QStyle>
#include <QSettings>
#include <QMenu>

TeDriveBar::TeDriveBar(QWidget *parent)
	: QToolBar(parent)
{
	setIconSize(QSize(16, 16));
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	{
		//Root directory
		QAction* act = new QAction(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon), "A");
		act->setData(QDir::rootPath());
		act->setToolTip(tr("Computer"));
		addAction(act);
		connect(act, &QAction::triggered, [this, act](bool) { selectDrive(act->data().toString()); });
	}
	{
		//Home directory
		QAction* act = new QAction(QApplication::style()->standardIcon(QStyle::SP_DesktopIcon), "B");
		act->setToolTip(tr("Home"));
		act->setData(QDir::homePath());
		addAction(act);
		connect(act, &QAction::triggered, [this, act](bool) { selectDrive(act->data().toString()); });

		mp_driveStart = act;
	}

	updateDrive(true);

	mp_quickStart = addSeparator();

	loadQuickAccesses();

	updateKey('L');
}

TeDriveBar::TeDriveBar(const QString &title, QWidget *parent)
	: TeDriveBar(parent)
{
	setWindowTitle(title);
}

TeDriveBar::~TeDriveBar()
{
}

void TeDriveBar::updateKey(const QChar& c)
{
	auto actions = this->actions();

	if (c != m_key) {
		//clear all shortcut
		for (QAction* action : actions) {
			action->setShortcut(QKeySequence());
		}
		m_key = c;
	}

	//set new shortcut
	if(!c.isNull()){
		for (QAction* action : actions) {
			if (!action->isSeparator()) {
				QString key = QString(c) + "," + action->text().left(1);
				action->setShortcut(QKeySequence(key));
			}
		}
	}
}

void TeDriveBar::loadQuickAccesses()
{
	//clear all quick access
	for (QAction* action : m_quickActions) {
		removeAction(action);
		delete action;
	}
	m_quickActions.clear();

	QFileIconProvider provider;

	QSettings settings;
	settings.beginGroup("QuickAccess");
	QStringList keys = settings.childKeys();
	for (const auto& key : keys) {
		QFileInfo info(settings.value(key).toString());
		QAction* act = new QAction(provider.icon(info), key + ": " + info.fileName());
		act->setData(info.filePath());
		act->setToolTip(info.filePath());
		addAction(act);
		connect(act, &QAction::triggered, [this, act](bool) { selectDrive(act->data().toString()); });
		m_quickActions.append(act);
	}
}

void TeDriveBar::storeQuickAccesses()
{
	QSettings settings;
	settings.beginGroup("QuickAccess");
	for (int i = 0; i < m_quickActions.size(); i++) {
		settings.setValue(QString::number(i+1), m_quickActions[i]->data().toString());
	}
	settings.endGroup();
}

void TeDriveBar::addQuickAccess(const QString& path)
{
	QFileInfo info(path);
	if (info.exists()) {
		QFileIconProvider provider;
		QAction* act = new QAction(provider.icon(info), QString::number(m_quickActions.size()+1) + ": " + info.fileName());
		act->setData(info.filePath());
		act->setToolTip(info.filePath());
		addAction(act);
		QString path = act->data().toString();
		connect(act, &QAction::triggered, [this, act](bool) { selectDrive(act->data().toString()); });

		m_quickActions.append(act);
		updateKey(m_key);
	}
}

void TeDriveBar::removeQuickAccess(const QString& path)
{
	//remove quick access
	for (int i = 0; i < m_quickActions.size(); i++) {
		if (m_quickActions[i]->data().toString() == path) {
			removeAction(m_quickActions[i]);
			delete m_quickActions[i];
			m_quickActions.removeAt(i);
			updateKey(m_key);
			break;
		}
	}

	//update number of quick access
	for (int i = 0; i < m_quickActions.size(); i++) {
		m_quickActions[i]->setText(QString::number(i + 1) + ": " + QFileInfo(m_quickActions[i]->data().toString()).fileName());
	}

	updateKey(m_key);
}

void TeDriveBar::setQuickAccess(const QStringList& paths)
{
	//clear all quick access
	for (QAction* action : m_quickActions) {
		removeAction(action);
		delete action;
	}
	m_quickActions.clear();
	
	QFileIconProvider provider;
	for (const auto& path : paths) {
		QFileInfo info(path);
		QAction* act = new QAction(provider.icon(info), QString::number(m_quickActions.size() + 1) + ": " + info.fileName());
		act->setData(info.filePath());
		act->setToolTip(info.filePath());
		addAction(act);
		connect(act, &QAction::triggered, [this, act](bool) { selectDrive(act->data().toString()); });
		m_quickActions.append(act);
	}
	
	updateKey(m_key);
}

QStringList TeDriveBar::quickAccess() const
{
	QStringList paths;
	for (const auto& act : m_quickActions) {
		paths.append(act->data().toString());
	}
	return paths;
}

void TeDriveBar::updateDrive(bool state)
{
	QFileIconProvider provider;
	//clear all drive
	for (QAction* action : m_driveActions) {
		removeAction(action);
		delete action;
	}
	m_driveActions.clear();

	for (const auto& drive : QStorageInfo::mountedVolumes()) {
		QString name = drive.displayName();
		if (name.isEmpty()) {
			name = drive.rootPath();
		}
		QAction* act = new QAction(provider.icon(QFileInfo(drive.rootPath())), drive.rootPath().left(1));
		act->setData(drive.rootPath());
		act->setToolTip(name);
		addAction(act);
		connect(act, &QAction::triggered, [this, act](bool) { selectDrive(act->data().toString()); });
		m_driveActions.append(act);
	}

	updateKey(m_key);
}

void TeDriveBar::selectDrive(const QString& path)
{
	QStorageInfo info(path);
	if (info.isReady()) {
		//emit new drive.
		emit driveSelected(path);
	}
}

