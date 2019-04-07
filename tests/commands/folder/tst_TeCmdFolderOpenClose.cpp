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

