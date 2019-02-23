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
#include "TeTreeWidget.h"

#include <QDragEnterEvent>;

TeTreeWidget::TeTreeWidget(QWidget *parent)
	: QTreeWidget(parent)
{
	mp_buddy = nullptr;
}

TeTreeWidget::~TeTreeWidget()
{
}

void TeTreeWidget::setBuddy(QTreeWidget * p_buddy)
{
	mp_buddy = p_buddy;
}

void TeTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
	if (event->source() == this || event->source() == mp_buddy) {
		//only accept from internal or buddy
		QTreeWidget::dragEnterEvent(event);
	}
	else {
		event->ignore();
	}
}

void TeTreeWidget::dropEvent(QDropEvent * event)
{
	if ( (mp_buddy != nullptr) &&  (event->source() == mp_buddy) ) {
		//Copy from buddy
		QTreeWidgetItem* item = mp_buddy->currentItem();
		if (item == nullptr) {
			event->ignore();
		}
		else {
			QModelIndex index = indexAt(event->pos());
			QTreeWidgetItem* cloneItem = item->clone();
			int row = -1;
			int col = -1;
			switch (dropIndicatorPosition()) {
			case OnItem:
			case OnViewport:
				break;
			case AboveItem:
				row = index.row();
				col = index.column();
				index = index.parent();
				break;
			case BelowItem:
				row = index.row() + 1;
				col = index.column();
				index = index.parent();
				break;
			}
			if (row == -1) {
				if (index.isValid()) {
					QTreeWidgetItem* parent = itemFromIndex(index);
					parent->insertChild(parent->childCount(), cloneItem);
				}
				else {
					insertTopLevelItem(topLevelItemCount(), cloneItem);
				}
			}
			else {
				QPersistentModelIndex dropRow = model()->index(row, col, index);
				int r = dropRow.row() >= 0 ? dropRow.row() : row;
				if (index.isValid()) {
					QTreeWidgetItem* parent = itemFromIndex(index);
					parent->insertChild(qMin(r,parent->childCount()), cloneItem);
				}
				else {
					insertTopLevelItem(qMin(r, topLevelItemCount()), cloneItem);
				}
			}
			event->accept();
			event->setDropAction(Qt::CopyAction);
		}
	}
	else if(event->source() == this){
		//internal move or copy
	}
	else {
		event->ignore();
	}
	QTreeWidget::dropEvent(event);
}
