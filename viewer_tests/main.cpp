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

#include <QApplication>
#include <gmock/gmock.h>

#include <TeDocViewer.h>

int main(int argc, char *argv[])
{
	::testing::InitGoogleMock(&argc, argv);

	QApplication a(argc, argv);
	a.setAttribute(Qt::AA_Use96Dpi, true);

#if 1
	TeDocViewer view;
	
	view.show();

	return a.exec();
#else
	return RUN_ALL_TESTS();
#endif
}
