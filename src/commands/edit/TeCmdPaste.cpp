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
#include "platform/TeFileOperationManager.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeArchiveFolderView.h"
#include "utils/TeUtils.h"
#include "platform/platform_util.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QMimeData>
#include <QList>
#include <QUrl>

/**
 * @file TeCmdPaste.cpp
 * @brief Implementation of TeCmdPaste.
 * @ingroup commands
 */

TeCmdPaste::TeCmdPaste()
{
}

TeCmdPaste::~TeCmdPaste()
{
}

bool TeCmdPaste::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdPaste::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_SELECT

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_OTHER

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_DIRECTORY
	);
}

bool TeCmdPaste::execute(TeViewStore* p_store)
{
	QString dstPath = p_store->currentFolderView()->currentPath();

	const QClipboard* clipboard = QGuiApplication::clipboard();
	const QMimeData* mime = clipboard->mimeData();

	if (mime == nullptr || !mime->hasUrls()) {
		return true;
	}

	QList<QUrl> urls = mime->urls();
	QStringList paths;
	for (const auto& url : urls) {
		if (url.isLocalFile()) {
			paths.append(url.toLocalFile());
		}
	}
	if (paths.isEmpty()) {
		return true;
	}

	TeArchiveFolderView* p_arc = qobject_cast<TeArchiveFolderView*>(p_store->currentFolderView());
	if (p_arc != nullptr) {
		// Paste into an archive is only supported in writable mode; it stages
		// the source files instead of touching the filesystem. The destination
		// may be the archive root, where the virtual path is empty.
		if (!p_arc->isReadOnly()) {
			p_arc->stageEntries(dstPath, paths);
		}
		return true;
	}

	if (dstPath.isEmpty()) {
		return true;
	}

	if (isMoveAction(mime)) {
		p_store->fileOperationManager()->moveFiles(paths, dstPath);
	}
	else {
		p_store->fileOperationManager()->copyFiles(paths, dstPath);
	}
	return true;
}
