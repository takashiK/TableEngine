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

#include <QDialog>
#include <QList>

#include "utils/TeFileInfo.h"

/**
 * @file TePropertiesDialog.h
 * @brief Declaration of TePropertiesDialog.
 * @ingroup dialogs
 */

/**
 * @class TePropertiesDialog
 * @brief Common, non-native properties dialog for filesystem and archive entries.
 * @ingroup dialogs
 *
 * @details Presents the unified attributes of one or more entries described by
 * TeFileInfo.  Fields that are unavailable for a given source (e.g. permissions
 * for some archive formats) are shown as a label with an empty value rather than
 * being populated, so the layout stays consistent across sources.
 */
class TePropertiesDialog : public QDialog
{
	Q_OBJECT

public:
	/**
	 * @brief Constructs the dialog for the supplied entries.
	 * @param items      Entries to describe.
	 * @param filesystem true when @p items originate from the local filesystem.
	 * @param parent     Parent widget.
	 */
	TePropertiesDialog(const QList<TeFileInfo>& items, bool filesystem, QWidget* parent = nullptr);
	~TePropertiesDialog();

	/** @brief Builds a list of TeFileInfo entries from filesystem paths. */
	static QList<TeFileInfo> fromPaths(const QStringList& paths);

private:
	void buildUi(const QList<TeFileInfo>& items, bool filesystem);
};
