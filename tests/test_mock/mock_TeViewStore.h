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

#pragma once

#include <TeViewStore.h>
#include <gmock/gmock.h>

class mock_TeViewStore : public TeViewStore {
public:
	MOCK_METHOD0(mainWindow,
		QMainWindow*());
	MOCK_METHOD0(currentTabIndex,
		int());
	MOCK_METHOD1(tabIndex,
		int(TeFileFolderView* view));
	MOCK_METHOD1(getFolderView,
		QList<TeFileFolderView*>(int));
	MOCK_METHOD0(currentFolderView,
		TeFileFolderView*());
	MOCK_METHOD1(setCurrentFolderView,
		void(TeFileFolderView* view));
	MOCK_METHOD2(createFolderView,
		TeFileFolderView*(QString, int));
	MOCK_METHOD1(deleteFolderView,
		void(TeFileFolderView* view));
	MOCK_METHOD3(moveFolderView,
		void(TeFileFolderView*, int, int));
};
