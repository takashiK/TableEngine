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

#include <QObject>
#include <TeArchive.h>

class FileEntry : public QObject, public TeArchive::FileInfo {
	Q_OBJECT
public:
	FileEntry(FileEntry* parent, const QString& name) : QObject(parent) { path = name; type = TeArchive::EN_DIR; }
	FileEntry(FileEntry* parent, const QString& name, qint64 size, const QDateTime& lastModified, const QString& src)
		: QObject(parent)
	{
		this->type = TeArchive::EN_FILE;
		this->path = name; this->size = size; this->lastModifyed = lastModified; this->src = src;
	}
};