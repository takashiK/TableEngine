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

#include "platform/platform_util.h"
#include "platform/TeNativeEvent.h"

#include <QDesktopServices>
#include <QUrl>
#include <QMimeData>
#include <QGuiApplication>
#include <QStyleHints>

/**
 * @file platform_util.cpp
 * @brief Linux (Ubuntu) implementation of platform_util. Shell integration
 *        features that require a desktop-specific file manager (context
 *        menus, thumbnails, file properties, IFileOperation-style copy/move)
 *        are not implemented and safely no-op / return false.
 * @ingroup platform
 */

namespace {
	TeNativeEvent g_event;
}

//////////////////////////////////////////////////////////////
//
// Initialize and uninitialize platform system
//
bool threadInitialize(QApplication* /*a*/)
{
	return true;
}

void threadUninitialize()
{
}

void comInitializeThread()
{
}

void comUninitializeThread()
{
}

//////////////////////////////////////////////////////////////
//
// File action
//
bool showFilesContext(int /*px*/, int /*py*/, const QStringList& /*paths*/)
{
	return false;
}

void showFileContext(int /*px*/, int /*py*/, const QString& /*path*/)
{
}

void showFilesProperties(const QStringList& /*paths*/)
{
}

void showFileProperties(const QString& /*path*/)
{
}

void openFile(const QString& path)
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool copyFiles(const QStringList& /*files*/, const QString& /*path*/, WId /*owner*/)
{
	return false;
}

bool copyFile(const QString& /*fromFile*/, const QString& /*toFile*/, WId /*owner*/)
{
	return false;
}

bool moveFiles(const QStringList& /*files*/, const QString& /*path*/, WId /*owner*/)
{
	return false;
}

bool deleteFiles(const QStringList& /*files*/, WId /*owner*/)
{
	return false;
}

QPixmap getThumbnail(const QString& /*path*/, const QSize& /*size*/)
{
	return QPixmap();
}

QPixmap getFileIcon(const QString& /*path*/, const QSize& /*size*/)
{
	return QPixmap();
}

QString getAssociatedAppPath(const QString& /*suffix*/)
{
	return QString();
}

//////////////////////////////////////////////////////////////
//
// Clipboard action
//
bool isMoveAction(const QMimeData* mime)
{
	if (mime->hasFormat("application/x-qt-windows-mime;value=\"Preferred DropEffect\"")) {
		const QByteArray array = mime->data("application/x-qt-windows-mime;value=\"Preferred DropEffect\"");
		return !array.isEmpty() && (array.at(0) == 2);
	}

	if (mime->hasFormat("Preferred DropEffect")) {
		const QByteArray array = mime->data("Preferred DropEffect");
		return !array.isEmpty() && (array.at(0) == 2);
	}

	return false;
}

void setMoveAction(QMimeData* mime)
{
	QByteArray data(4, 0);
	data[0] = 2;
	mime->setData("Preferred DropEffect", data);
}

void setCopyAction(QMimeData* mime)
{
	QByteArray data(4, 0);
	data[0] = 5;
	mime->setData("Preferred DropEffect", data);
}

//////////////////////////////////////////////////////////////
//
// NativeEventFilter
//
TeNativeEvent* getNativeEvent()
{
	return &g_event;
}

//////////////////////////////////////////////////////////////
//
// Shell
//
QString getDefaultShellCommand()
{
	return QStringLiteral("/bin/sh");
}

//////////////////////////////////////////////////////////////
//
// Colour scheme
//
TeStyleColorScheme getStyleColorScheme()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
	const QStyleHints* hints = QGuiApplication::styleHints();
	if (hints) {
		switch (hints->colorScheme()) {
		case Qt::ColorScheme::Dark:
			return TeStyleColorScheme::Dark;
		case Qt::ColorScheme::Light:
			return TeStyleColorScheme::Light;
		default:
			break;
		}
	}
#endif
	return TeStyleColorScheme::Light;
}
