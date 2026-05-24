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

#include "TeCmdTreeWidget.h"

#include <QTreeWidgetItem>

/**
 * @file TeCmdTreeWidget.cpp
 * @brief QTreeWidget subclass used as drag source for command items.
 * @ingroup dialogs
 */

const QString TeCmdTreeWidget::MimeType = QStringLiteral("application/x-tableengine-command");

TeCmdTreeWidget::TeCmdTreeWidget(QWidget* parent)
	: QTreeWidget(parent)
{
	setDragDropMode(QAbstractItemView::DragOnly);
}

QStringList TeCmdTreeWidget::mimeTypes() const
{
	return { MimeType };
}

/**
 * Returns a TeCmdMimeData holding the leaf item's three data roles as QVariants.
 * Using an in-memory carrier avoids QDataStream round-trips, which would produce
 * a null QIcon for icons backed by a custom QIconEngine (e.g. TeAdaptiveIconEngine)
 * that does not implement write()/read().
 * Root category items (columnCount == 1) are skipped.
 */
QMimeData* TeCmdTreeWidget::mimeData(const QList<QTreeWidgetItem*>& items) const
{
	for (const QTreeWidgetItem* item : items) {
		if (item->columnCount() != 3)
			continue;

		TeCmdMimeData* data = new TeCmdMimeData();
		data->name  = item->data(0, Qt::DisplayRole);
		data->icon  = item->data(0, Qt::DecorationRole);
		data->cmdId = item->data(2, Qt::DisplayRole);
		return data;
	}
	return nullptr;
}
