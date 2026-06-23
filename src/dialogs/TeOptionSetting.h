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
#include <QHash>
#include <QVariant>
#include "../utils/TeFolderAppearance.h"

/**
 * @file TeOptionSetting.h
 * @brief Declaration of TeOptionSetting.
 * @ingroup dialogs
 */

class TeFileListView;

class TeOptionSetting : public QDialog
{
	Q_OBJECT

public:
	/**
	 * @param parent    Parent widget.
	 * @param p_listView Optional TeFileListView whose stylesheet-polished palette
	 *                   is used as fallback when a setting has not been saved yet.
	 */
	TeOptionSetting(QWidget *parent, TeFileListView* p_listView = nullptr);
	virtual ~TeOptionSetting();
	
	void updateSettings();
	static void storeDefaultSettings(bool force=false);

public slots:
	void accept();

protected:
	QWidget* createPageGeneral();
	QWidget* createPageFolder();
	QWidget* createPageWindow();
	QWidget* createPagePanel();
	TeFolderAppearance loadFolderAppearance(bool useSettings=true) const;
	QHash<QString, QVariant> m_option;
	TeFolderAppearance       m_folderAppearance;
	TeFileListView*          mp_listView = nullptr;
};
