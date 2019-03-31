/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 2 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL2
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

#include <QDialog>
class QComboBox;

class TeFilePathDialog : public QDialog
{
	Q_OBJECT

public:
	TeFilePathDialog(QWidget *parent = nullptr);
	virtual  ~TeFilePathDialog();

	void setCurrentPath(const QString& path);
	QString currentPath();

	void setTargetPath(const QString& path);
	QString targetPath();

//	void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	QComboBox* mp_combo;
	QString    m_currentPath;
};
