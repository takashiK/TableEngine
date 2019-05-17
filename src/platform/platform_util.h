/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
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
