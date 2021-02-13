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

#include <gmock/gmock.h>
#include <test_mock/mock_TeViewStore.h>
#include <test_mock/mock_TeDispatcher.h>

using ::testing::Return;

#include <widgets/TeFileFolderView.h>

#include <commands/folder/TeCmdFolderCloseUnder.h>
#include <commands/folder/TeCmdFolderOpenAll.h>
#include <commands/folder/TeCmdFolderOpenOne.h>
#include <commands/folder/TeCmdFolderOpenUnder.h>

TEST(tst_TeCmdFolderOpenClose, openOneNormal)
{
	mock_TeDispatcher dispatcher;
	mock_TeViewStore store;
	TeFileFolderView folderview;

	TeCmdFolderOpenOne cmd;

	EXPECT_CALL(store, currentFolderView())
		.WillOnce(Return(&folderview));
	EXPECT_CALL(dispatcher, requestCommandFinalize(&cmd))
		.Times(1);

	cmd.setDispatcher(&dispatcher);

	cmd.run(&store);
}

