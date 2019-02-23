/****************************************************************************
**
** Copyright (C) 2018 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
** This file is part of the Table Engine.
**
** $QT_BEGIN_LICENSE:GPL$
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public license version 3 or any later version.
** The licenses are as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "TeCmdFolderOpenAll.h"

#include "TeViewStore.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeFileTreeView.h"



TeCmdFolderOpenAll::TeCmdFolderOpenAll()
{
}


TeCmdFolderOpenAll::~TeCmdFolderOpenAll()
{
}

bool TeCmdFolderOpenAll::execute(TeViewStore * p_store)
{
	TeFileFolderView* p_folder = p_store->currentFolderView();

	if (p_folder != nullptr) {
		TeFileTreeView* p_tree = p_folder->tree();
		p_tree->expandAll();
	}

	return true;
}
