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
#include "TeTypes.h"

class QRubberBand;
class TeFolderView;

class TeFileListView : public QListView
{
	Q_OBJECT

public:

public:
	TeFileListView(QWidget *parent = Q_NULLPTR);
	virtual  ~TeFileListView();

	virtual TeFolderView* folderView();
	void setFolderView(TeFolderView* view);

	void setSelectionMode(TeTypes::SelectionMode mode);
	TeTypes::SelectionMode selectionMode() const;

protected:
	virtual void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual QItemSelectionModel::SelectionFlags selectionCommand(const QModelIndex& index, const QEvent* event = Q_NULLPTR) const;

private:
	TeFolderView* mp_folderView;
	bool m_clearAndSelect_by_press;

	QModelIndex m_pressedIndex;
	QPoint m_pressedPos;
	QRubberBand* mp_rubberBand;

	TeTypes::SelectionMode m_selectionMode;
};
