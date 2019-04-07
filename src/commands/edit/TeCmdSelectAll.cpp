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
#include "TeCmdSelectAll.h"
#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileListView.h"

#include <QAbstractItemModel>

TeCmdSelectAll::TeCmdSelectAll()
{
}


TeCmdSelectAll::~TeCmdSelectAll()
{
}

/**
 * カレントのTeFileListView上での全選択/解除を実施する。
 */
bool TeCmdSelectAll::execute(TeViewStore* p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileListView* p_list = p_folder->list();

		if (p_list->selectionModel()->hasSelection()) {
			//選択済みなら、全解除
			p_list->clearSelection();
		}
		else {
			//なにも選択されていなければ、全選択
			p_list->selectAll();
		}
	}

	return true;
}