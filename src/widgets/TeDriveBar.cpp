/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "TeDriveBar.h"

#include <QFileIconProvider>
#include <QDir>
#include <QStorageInfo>
#include <QFileSystemWatcher>

TeDriveBar::TeDriveBar(QWidget *parent)
	: QToolBar(parent)
{
	mp_current = nullptr;
	mp_watcher = new QFileSystemWatcher();
	setIconSize(QSize(16, 16));
	setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	QFileIconProvider provider;

	QFileInfoList drives = QDir::drives();
	for (QFileInfoList::const_iterator itrDrive = drives.cbegin(); itrDrive != drives.cend(); ++itrDrive) {
		QAction* act = new QAction(provider.icon(*itrDrive), itrDrive->path().left(1));
		act->setCheckable(true);
		act->setData(itrDrive->path());
		addAction(act);
		connect(act, &QAction::triggered, [this,act](bool checked) { selectDrive(act,checked); });
	}

	//For test
	QFileInfo info(u8"C:/Qt");
	QAction* act = new QAction(provider.icon(info), info.fileName());
	act->setCheckable(true);
	act->setData(info.filePath());
	addAction(act);
	QString path = act->data().toString();
	connect(act, &QAction::triggered, [this, act](bool checked) { selectDrive(act, checked); });

}

TeDriveBar::TeDriveBar(const QString &title, QWidget *parent)
	: TeDriveBar(parent)
{
	setWindowTitle(title);
}

TeDriveBar::~TeDriveBar()
{
}

void TeDriveBar::driveChanged(const QString & path)
{
	int maxCount = 0;
	QAction* act = nullptr;
	for (QAction* action : actions()) {
		if (action->data().toString().contains(path)) {
			if (maxCount < path.count()) {
				maxCount = path.count();
				act = action;
			}
		}
	}

	selectDrive(act, true);
}

void TeDriveBar::selectDrive(QAction * act, bool checked)
{
	if (act == nullptr) {
		for (QAction* action : actions()) {
			action->setChecked(false);
		}
	}
	else if (act == mp_current ) {
		//if action is "Release check" then rechecked
		if (!checked) {
			act->setChecked(true);
		}
	}
	else {
		QStorageInfo info(act->data().toString());
		if (!info.isReady()) {
			//if drive is not ready then force canceled.
			act->setChecked(false);
		}
		else {
			//if checked drive is ready then other drive release check.
			for (QAction* action : actions()) {
				if (action != act) {
					action->setChecked(false);
				}
			}
			mp_current = act;

			//emit new drive.
			QString path = mp_current->data().toString();
			emit changeDrive(mp_current->data().toString());
		}
	}
}
