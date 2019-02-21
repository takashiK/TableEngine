#include "TeFileListView.h"

#include <QApplication>
#include <QKeyEvent>

TeFileListView::TeFileListView(QWidget *parent)
	: QListView(parent)
{
	mp_folderView = nullptr;
}

TeFileListView::~TeFileListView()
{
}

TeFileFolderView * TeFileListView::folderView()
{
	return mp_folderView;
}

void TeFileListView::setFolderView(TeFileFolderView * view)
{
	mp_folderView = view;
}

/**
 * キー押下時の振舞いを決める
 */
void TeFileListView::keyPressEvent(QKeyEvent *event)
{
	QListView::keyPressEvent(event);

	QPersistentModelIndex newCurrent, current;
	Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
	if (event) {
		switch (event->type()) {
		case QEvent::KeyPress: {
			modifiers = static_cast<const QKeyEvent*>(event)->modifiers();
			switch (static_cast<const QKeyEvent*>(event)->key()) {
			case Qt::Key_Space:
				//スペースキー押下後にカーソルを次のアイテムに移動させる
				current = currentIndex();
				newCurrent = model()->index(current.row() + 1, current.column(), rootIndex());
				if (newCurrent.isValid()) {
					selectionModel()->setCurrentIndex(newCurrent, QItemSelectionModel::NoUpdate);
				}
			}
			break;
		}
		default:
			break;
		}
	}
}

/**
 * イベントに対する選択状態の変化パターンを記述する関数
 */
QItemSelectionModel::SelectionFlags TeFileListView::selectionCommand(const QModelIndex &index, const QEvent *event) const
{
	if (selectionMode() == QListView::ExtendedSelection) {
		Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
		if (event == nullptr) {
			//setCurrentIndexで Clear & Selectが発動しないようにする。
			return QItemSelectionModel::NoUpdate;
		}else{
			switch (event->type()) {
			case QEvent::MouseButtonRelease:
			case QEvent::MouseButtonPress: {
				modifiers = static_cast<const QMouseEvent*>(event)->modifiers();
				const bool shiftKeyPressed = modifiers & Qt::ShiftModifier;
				const bool controlKeyPressed = modifiers & Qt::ControlModifier;
				if (!shiftKeyPressed && !controlKeyPressed) {
					//マウスクリックによる選択動作をさせない (Spaceキーでのトグル動作と相性が悪いため)
					return QItemSelectionModel::NoUpdate;
				}
				break;
			}
			case QEvent::KeyPress: {
				switch (static_cast<const QKeyEvent*>(event)->key()) {
				case Qt::Key_Down:
				case Qt::Key_Up:
				case Qt::Key_Left:
				case Qt::Key_Right:
				case Qt::Key_Home:
				case Qt::Key_End:
				case Qt::Key_PageUp:
				case Qt::Key_PageDown:
					//移動キー押下時に選択が解除されるのを防ぐ
					return QItemSelectionModel::NoUpdate;
				case Qt::Key_Space:
					return QItemSelectionModel::Toggle;
				}
			}
			default:
				//デフォルトはQtの規定動作とする。
				return QListView::selectionCommand(index, event);
			}
		}
	}
	return QListView::selectionCommand(index, event);
}
