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

#include <QTreeWidget>
#include <QMimeData>
#include <QVariant>

/**
 * @file TeCmdTreeWidget.h
 * @brief QTreeWidget subclass used as drag source for command items.
 * @ingroup dialogs
 */

class TeCmdTreeWidget : public QTreeWidget
{
	Q_OBJECT

public:
	/// Custom MIME type used to transfer command data between the two tree widgets.
	static const QString MimeType;

	explicit TeCmdTreeWidget(QWidget* parent = nullptr);

protected:
	QStringList mimeTypes() const override;
	QMimeData*  mimeData(const QList<QTreeWidgetItem*>& items) const override;
};

/**
 * @class TeCmdMimeData
 * @brief Custom QMimeData carrier for command drag operations.
 *
 * Holds the three data roles (name, icon, cmdId) as QVariants directly in
 * memory instead of serialising them through QDataStream.  This avoids the
 * QIconEngine serialisation limitation: custom engines such as
 * TeAdaptiveIconEngine do not implement write()/read(), so a round-trip
 * through QDataStream produces a null QIcon.  Because drag-and-drop within
 * the same process never leaves the process boundary, no byte-level
 * serialisation is required.
 */
class TeCmdMimeData : public QMimeData
{
public:
	QVariant name;
	QVariant icon;
	QVariant cmdId;

	QStringList formats() const override { return { TeCmdTreeWidget::MimeType }; }
	bool hasFormat(const QString& mt) const override { return mt == TeCmdTreeWidget::MimeType; }
};
