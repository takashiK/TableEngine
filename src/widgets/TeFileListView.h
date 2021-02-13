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
