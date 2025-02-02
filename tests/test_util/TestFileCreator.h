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

#include <QString>
#include <QDateTime>

#include "TeFileFinder.h"
#include "TeFileInfo.h"
#include <QStandardItem>

void createFileTree(const QString& dest, const QStringList& paths, int extend_mbytes=0);
void cleanFileTree(const QString& path);
bool compareFileTree(const QString& src, const QString& dst, bool binComp);
void expectEntries(QStandardItem* root, const QStringList& paths, const QDateTime& date = QDateTime());
bool compareFileTree(const QStandardItem* srcRoot, const QString& dst, bool binComp);

enum FileInfoUserRole {
	ROLE_FINFO_TYPE = Qt::UserRole,
	ROLE_FINFO_PATH,
	ROLE_FINFO_SIZE,
	ROLE_FINFO_DATE,
	ROLE_FINFO_PERM,
};

void convertToStandardItem(const TeFileInfo& info, QStandardItem* item);
void convertToFileInfo(const QStandardItem* item, TeFileInfo* info);
