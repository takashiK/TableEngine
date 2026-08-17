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
#include <QList>
#include <QPixmap>
#include <QSize>
#include <QApplication>

/**
 * @file platform_util.h
 * @brief Declaration of platform_util.
 * @ingroup platform
 */


class QMimeData;
class TeNativeEvent;
class TeFileOpProgress;

// Platform-neutral colour scheme, kept independent of Qt::ColorScheme so this
// header compiles unchanged against Qt versions that lack that API (< 6.5).
enum class TeStyleColorScheme {
	Light,
	Dark
};

struct TeDriveAction {
	QString text;
	QString path;
	QString toolTip;
};

//Initialize and uninitialize platform system
extern bool threadInitialize(QApplication* a);
extern void threadUninitialize();

//Initialize and uninitialize COM (or equivalent) for a worker thread.
//Used by background file-operation threads so shell APIs can be called off
//the GUI thread.  No-op on platforms that do not require per-thread init.
extern void comInitializeThread();
extern void comUninitializeThread();

//File action
extern bool showFilesContext(int px, int py, const QStringList& paths);
extern void showFileContext( int px, int py, const QString& path);

extern void showFilesProperties(const QStringList& paths);
extern void showFileProperties(const QString& path);

extern void openFile(const QString& path);

// The optional TeFileOpProgress* lets a background worker thread report
// processed/total bytes and the current path to a GUI-thread progress
// dialog without blocking cross-thread calls (see TeFileOpProgress.h).
// Callers that don't need progress reporting can omit it.
extern bool copyFiles(const QStringList& files, const QString& path, WId owner = 0, TeFileOpProgress* progress = nullptr);
extern bool copyFile(const QString& fromFile, const QString& toFile, WId owner = 0, TeFileOpProgress* progress = nullptr);

extern bool moveFiles(const QStringList& files, const QString& path, WId owner = 0, TeFileOpProgress* progress = nullptr);

extern bool deleteFiles(const QStringList& files, WId owner = 0, TeFileOpProgress* progress = nullptr);

extern QPixmap getThumbnail(const QString& path, const QSize& size);
extern QPixmap getFileIcon(const QString& path, const QSize& size);

extern QString getAssociatedAppPath(const QString& suffix);

//clipboard action
extern bool isMoveAction(const QMimeData* mime);
// NOTE: mime->setUrls() must be called before setMoveAction()/setCopyAction();
// the Linux implementation reads the URL list to build GNOME/KDE clipboard formats.
extern void setMoveAction(QMimeData* mime);
extern void setCopyAction(QMimeData* mime);

//NativeEventFilter
extern TeNativeEvent* getNativeEvent();

//shell
// OS-specific fallback shell command, used when the SHELL environment variable is not set.
extern QString getDefaultShellCommand();

extern QList<TeDriveAction> getDriveActions();

#ifdef Q_OS_LINUX
namespace platform_util_test {
QString driveActionLabel(int index);
bool isExternalBlockDevice(const QString& blockDevice, const QString& sysfsRoot);
}
#endif

// Current system/application colour scheme. Returns TeStyleColorScheme::Light
// when detection is unavailable (e.g. Qt < 6.5 or no platform support).
extern TeStyleColorScheme getStyleColorScheme();
