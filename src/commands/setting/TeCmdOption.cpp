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

#include "TeCmdOption.h"
#include "TeViewStore.h"
#include "dialogs/TeOptionSetting.h"
#include <QDialog>

TeCmdOption::TeCmdOption()
{
}


TeCmdOption::~TeCmdOption()
{
}

bool TeCmdOption::execute(TeViewStore * p_store)
{
	TeOptionSetting dlg(p_store->mainWindow());
	if (dlg.exec() == QDialog::Accepted) {
		//設定を反映
		p_store->loadSetting();
	}
	return true;
}
