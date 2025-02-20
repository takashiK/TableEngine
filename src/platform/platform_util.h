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
#include <QPixmap>
#include <QSize>
#include <QApplication>

class QMimeData;
class TeNativeEvent;

//Initialize and uninitialize platform system
extern bool threadInitialize(QApplication* a);
extern void threadUninitialize();

//File action
extern bool showFilesContext(int px, int py, const QStringList& paths);
extern void showFileContext( int px, int py, const QString& path);

extern void showFilesProperties(const QStringList& paths);
extern void showFileProperties(const QString& path);

extern void openFile(const QString& path);

extern bool copyFiles(const QStringList& files, const QString& path);
extern bool copyFile(const QString& fromFile, const QString& toFile);

extern bool moveFiles(const QStringList& files, const QString& path);

extern bool deleteFiles(const QStringList& files);

extern QPixmap getThumbnail(const QString& path, const QSize& size);
extern QPixmap getFileIcon(const QString& path, const QSize& size);

//clipboard action
extern bool isMoveAction(const QMimeData* mime);
extern void setMoveAction(QMimeData* mime);
extern void setCopyAction(QMimeData* mime);

//NativeEventFilter
extern TeNativeEvent* getNativeEvent();
