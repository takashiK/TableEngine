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

#include "TeKeyTreeWidget.h"
#include "TeCmdTreeWidget.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QTreeWidgetItem>

/**
 * @file TeKeyTreeWidget.cpp
 * @brief QTreeWidget subclass used as drop target for the key assignment list.
 * @ingroup dialogs
 */

TeKeyTreeWidget::TeKeyTreeWidget(QWidget* parent)
	: QTreeWidget(parent)
{
	setDragDropMode(QAbstractItemView::DropOnly);
	setDropIndicatorShown(false);
}

void TeKeyTreeWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(TeCmdTreeWidget::MimeType)) {
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

/**
 * Accepts the drag only when hovering over a leaf item (columnCount == 3).
 * Hovering over a root category row or an empty area is rejected so the
 * cursor gives clear feedback about valid drop targets.
 */
void TeKeyTreeWidget::dragMoveEvent(QDragMoveEvent* event)
{
	if (!event->mimeData()->hasFormat(TeCmdTreeWidget::MimeType)) {
		event->ignore();
		return;
	}

	QTreeWidgetItem* target = itemAt(event->position().toPoint());
	if (target && target->columnCount() == 3) {
		event->acceptProposedAction();
	} else {
		event->ignore();
	}
}

/**
 * Overwrites the key item's command association with the dropped command data.
 * The three QVariants packed by TeCmdTreeWidget::mimeData() are deserialized
 * and written to columns 1 (name + icon) and 2 (cmdId) of the target item —
 * identical to the itemActivated double-click logic.
 */
void TeKeyTreeWidget::dropEvent(QDropEvent* event)
{
	if (!event->mimeData()->hasFormat(TeCmdTreeWidget::MimeType)) {
		event->ignore();
		return;
	}

	QTreeWidgetItem* target = itemAt(event->position().toPoint());
	if (!target || target->columnCount() != 3) {
		event->ignore();
		return;
	}

	const TeCmdMimeData* cmdData = static_cast<const TeCmdMimeData*>(event->mimeData());
	target->setData(1, Qt::DisplayRole,    cmdData->name);
	target->setData(1, Qt::DecorationRole, cmdData->icon);
	target->setData(2, Qt::DisplayRole,    cmdData->cmdId);

	event->acceptProposedAction();
}
