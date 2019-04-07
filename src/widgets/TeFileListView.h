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
** Public license version 3 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#pragma once

#include <QListView>

class TeFolderView;

class TeFileListView : public QListView
{
	Q_OBJECT

public:
	TeFileListView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileListView();

	virtual TeFolderView* folderView();
	void setFolderView(TeFolderView* view);

protected:
	virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex &index,	const QEvent *event = Q_NULLPTR) const;
	virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

private:
	TeFolderView* mp_folderView;
};
