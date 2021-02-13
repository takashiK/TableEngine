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

#include "TeFolderView.h"
#include "TeFileTreeView.h"
#include "TeFileListView.h"
#include "TeEventFilter.h"

#include <QLayout>
#include <QHeaderView>
#include <QKeyEvent>
#include <QStorageInfo>


TeFolderView::TeFolderView(QWidget *parent)
	: QWidget(parent)
{
	mp_dispatcher = Q_NULLPTR;
}

TeFolderView::~TeFolderView()
{
}

void TeFolderView::setDispatcher(TeDispatchable * p_dispatcher)
{
	mp_dispatcher = p_dispatcher;
}

bool TeFolderView::dispatch(TeTypes::WidgetType type, QObject * obj, QEvent * event)
{
	if ((mp_dispatcher != nullptr) && isDispatchable(type, obj, event)) {
		return mp_dispatcher->dispatch(type, obj, event);
	}
	return false;
}

bool TeFolderView::isDispatchable(TeTypes::WidgetType /*type*/, QObject* /*obj*/, QEvent *event) const
{
	//select target event.
	if (event->type() == QEvent::KeyPress) {
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

		if (keyEvent->modifiers() == Qt::NoModifier || keyEvent->modifiers() == Qt::ShiftModifier || keyEvent->modifiers() == Qt::ControlModifier) {
			switch (keyEvent->key()) {
			case Qt::Key_F1:
			case Qt::Key_F2:
			case Qt::Key_F3:
			case Qt::Key_F4:
			case Qt::Key_F5:
			case Qt::Key_F6:
			case Qt::Key_F7:
			case Qt::Key_F8:
			case Qt::Key_F9:
			case Qt::Key_F10:
			case Qt::Key_F11:
			case Qt::Key_F12:
			case Qt::Key_Enter:
			case Qt::Key_Backspace:
			case Qt::Key_Delete:
				return true;

			case Qt::Key_0:
			case Qt::Key_1:
			case Qt::Key_2:
			case Qt::Key_3:
			case Qt::Key_4:
			case Qt::Key_5:
			case Qt::Key_6:
			case Qt::Key_7:
			case Qt::Key_8:
			case Qt::Key_9:

			case Qt::Key_A:
			case Qt::Key_B:
			case Qt::Key_C:
			case Qt::Key_D:
			case Qt::Key_E:
			case Qt::Key_F:
			case Qt::Key_G:
			case Qt::Key_H:
			case Qt::Key_I:
			case Qt::Key_J:
			case Qt::Key_K:
			case Qt::Key_L:
			case Qt::Key_M:
			case Qt::Key_N:
			case Qt::Key_O:
			case Qt::Key_P:
			case Qt::Key_Q:
			case Qt::Key_R:
			case Qt::Key_S:
			case Qt::Key_T:
			case Qt::Key_U:
			case Qt::Key_V:
			case Qt::Key_W:
			case Qt::Key_X:
			case Qt::Key_Y:
			case Qt::Key_Z:
				if (keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
					//Use Shift key to file selection.
					return false;
				}
				else {
					return true;
				}
			default:
				break;
			}
		}
	}
	return false;
}
