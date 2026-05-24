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

#include "TeCmdNaviDetailScrollDown.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"
#include "widgets/TeDetailView.h"

#include <QScrollArea>
#include <QScrollBar>

/**
 * @file TeCmdNaviDetailScrollDown.cpp
 * @brief Declaration of TeCmdNaviDetailScrollDown.
 * @ingroup commands
 */


TeCmdNaviDetailScrollDown::TeCmdNaviDetailScrollDown()
{
}

TeCmdNaviDetailScrollDown::~TeCmdNaviDetailScrollDown()
{
}

bool TeCmdNaviDetailScrollDown::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	NOT_USED(p_cmdParam);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdNaviDetailScrollDown::type()
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


bool TeCmdNaviDetailScrollDown::execute(TeViewStore* p_store)
{
	if (p_store && p_store->isDetailVisible()) {
		QScrollArea* detailScrollArea = p_store->detailView()->scrollArea();
		if (detailScrollArea) {
			QScrollBar* vScrollBar = detailScrollArea->verticalScrollBar();
			if (vScrollBar) {
				vScrollBar->setValue(vScrollBar->value() + vScrollBar->singleStep());
				return true;
			}
		}
	}
	return true;
}
