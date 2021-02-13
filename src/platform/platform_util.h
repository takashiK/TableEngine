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



extern void showFileContext( int px, int py, const QString& path);
extern void openFile(const QString& path);
extern bool copyFiles(const QStringList& files, const QString& path);
extern bool copyFile(const QString& fromFile, const QString& toFile);
extern bool moveFiles(const QStringList& files, const QString& path);
extern bool deleteFiles(const QStringList& files);

extern bool threadInitialize();
extern void threadUninitialize();
