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

#include "TeCmdPaste.h"
#include "platform/platform_util.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QList>
#include <QUrl>

TeCmdPaste::TeCmdPaste()
{
}

TeCmdPaste::~TeCmdPaste()
{
}

bool TeCmdPaste::isActive( TeViewStore* p_store )
{
	return false;
}

QFlags<TeTypes::CmdType> TeCmdPaste::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_TOGGLE
		// TeTypes::CMD_TRIGGER_SELECTION

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_NONE

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_ARCHIVE
	);
}

QList<TeMenuParam> TeCmdPaste::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdPaste::execute(TeViewStore* p_store)
{
	QString dstPath = p_store->currentFolderView()->currentPath();

	const QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mime = clipboard->mimeData();

	if (mime != nullptr && mime->hasUrls() && !dstPath.isEmpty()) {
		QList<QUrl> urls = mime->urls();
		QStringList paths;
		for (auto& url : urls) {
			if (url.isLocalFile()) {
				paths.append(url.toLocalFile());
			}
		}

		if (!paths.isEmpty()) {
			if (isMoveAction(mime)) {
				moveFiles(paths, dstPath);
			}
			else {
				copyFiles(paths, dstPath);
			}
		}
	}
	return true;
}
