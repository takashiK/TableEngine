/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
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
#include <QDateTime>
class FileEntry;

void createFileTree(const QString& dest, const QStringList& paths, int extend_mbytes =0);
void cleanFileTree(const QString& path);
bool compareFileTree(const QString& src, const QString& dst, bool binComp);
void expectEntries(FileEntry* root, const QStringList& paths, const QDateTime& date = QDateTime(), bool noDotDot = false);
bool compareFileTree(const FileEntry* srcRoot, const QString& dst, bool binComp);
