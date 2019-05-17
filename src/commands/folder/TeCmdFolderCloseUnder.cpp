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
#include "TeCmdFolderCloseUnder.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"


TeCmdFolderCloseUnder::TeCmdFolderCloseUnder()
{
}


TeCmdFolderCloseUnder::~TeCmdFolderCloseUnder()
{
}

bool TeCmdFolderCloseUnder::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		if (p_tree->currentIndex().isValid()) {
			p_tree->collapse(p_tree->currentIndex());
		}
	}

	return true;
}
