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
#include "TeViewStore.h"
#include "widgets/TeMainWindow.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeDriveBar.h"
#include "TeDispatcher.h"

#include <QMenu>
#include <QMenuBar>
#include <QFileIconProvider>
#include <QToolBar>
#include <QLabel>
#include <QStatusBar>
#include <QFileSystemModel>
#include <QHeaderView>
#include <QSplitter>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSettings>

TeViewStore::TeViewStore(QObject *parent)
	: QObject(parent)
{
	mp_dispatcher = nullptr;
	mp_mainWindow = nullptr;
	mp_tab[TAB_LEFT] = nullptr;
	mp_tab[TAB_RIGHT] = nullptr;
	mp_currentFolderView = nullptr;
	mp_split = nullptr;
}

TeViewStore::~TeViewStore()
{
	//Order of deletion is important.
	//TeFileTree is chiled of both QMainWindow and TeFolderView.
	//and relationship TeFileTree and TeFolderView is not depended "Qt Object System".
	//if QMainWindow is destroyed before TeFolderView then TeFileTree has double free.
	//Same things for QTab. It is child of both QMainWindow and TeViewStore.
	//
	//so we need delete QTab before QMainWindow.
	//TeFolderView is child of QTab. so this action do "delete TeFolderView before QMainWindow."
	if (mp_tab[TAB_LEFT]) delete mp_tab[TAB_LEFT];
	if (mp_tab[TAB_RIGHT]) delete mp_tab[TAB_RIGHT];
	if (mp_mainWindow) delete mp_mainWindow;
}

void TeViewStore::initialize()
{
	//Main window
	mp_mainWindow = new TeMainWindow;
	TeFileListView*   list = new TeFileListView;

	//Drive bar
	mp_driveBar = new TeDriveBar("Drive Bar");
	mp_mainWindow->addToolBar(mp_driveBar);

	//Status Bar
	QLabel *labelR = new QLabel(u8"Right Text");
	mp_mainWindow->statusBar()->addPermanentWidget(labelR);
	QLabel *labelL = new QLabel(u8"Left Text");
	mp_mainWindow->statusBar()->addWidget(labelL);

	//Add Tabs
	mp_tab[TAB_LEFT] = new QTabWidget();
	mp_tab[TAB_LEFT]->setMovable(true);
	mp_tab[TAB_LEFT]->setTabBarAutoHide(true);

	mp_tab[TAB_RIGHT] = new QTabWidget();
	mp_tab[TAB_RIGHT]->setMovable(true);
	mp_tab[TAB_RIGHT]->setHidden(true);

	QHBoxLayout *hbox = new QHBoxLayout();
	hbox->setMargin(0);
	hbox->addWidget(mp_tab[TAB_LEFT]);
	hbox->addWidget(mp_tab[TAB_RIGHT]);

	QWidget *folder_widget = new QWidget();
	folder_widget->setLayout(hbox);
	QSizePolicy listPolicy = folder_widget->sizePolicy();
	listPolicy.setHorizontalStretch(2);
	folder_widget->setSizePolicy(listPolicy);

	//Splitter
	mp_split = new QSplitter();
	mp_split->addWidget(folder_widget);

	mp_mainWindow->setCentralWidget(mp_split);

	connect(mp_tab[TAB_LEFT], &QTabWidget::currentChanged, [this](int index) {setCurrentFolderView(qobject_cast<TeFileFolderView*>(mp_tab[TAB_LEFT]->widget(index))); });
	connect(mp_tab[TAB_RIGHT], &QTabWidget::currentChanged, [this](int index) {setCurrentFolderView(qobject_cast<TeFileFolderView*>(mp_tab[TAB_RIGHT]->widget(index))); });

	//設定情報復帰
	loadMenu();
	loadSetting();
	loadStatus();
}

/*!
 * メニュー項目を設定ファイルから読みだして構築する。
 */
void TeViewStore::loadMenu()
{
	//Menu

	//File
	QMenu *menu = mp_mainWindow->menuBar()->addMenu(tr("&File"));
	menu->addAction(new QAction(tr("&New")));

	//Edit
	menu = mp_mainWindow->menuBar()->addMenu(tr("&Edit"));
	QAction* action = new QAction(QIcon(":/TableEngine/selectAll.png"), tr("Select &All"));
	connect(action, &QAction::triggered, [this](bool checked) { emit requestCommand(TeTypes::CMDID_SYSTEM_EDIT_SELECT_ALL, TeTypes::WT_NONE, nullptr, nullptr); });
	menu->addAction(action);

	action = new QAction(QIcon(":/TableEngine/selectToggle.png"), tr("&Toggle"));
	connect(action, &QAction::triggered, [this](bool checked) { emit requestCommand(TeTypes::CMDID_SYSTEM_EDIT_SELECT_TOGGLE, TeTypes::WT_NONE, nullptr, nullptr); });
	menu->addAction(action);

	//Setting
	menu = mp_mainWindow->menuBar()->addMenu(tr("&Setting"));
	action = new QAction(QIcon(":/TableEngine/settings.png"), tr("&Option"));
	connect(action, &QAction::triggered, [this](bool checked) {emit requestCommand(TeTypes::CMDID_SYSTEM_SETTING_OPTION, TeTypes::WT_NONE, nullptr, nullptr); });
	menu->addAction(action);

	action = new QAction(QIcon(":/TableEngine/keyboard.png"), tr("&Key"));
	connect(action, &QAction::triggered, [this](bool checked) {emit requestCommand(TeTypes::CMDID_SYSTEM_SETTING_KEY, TeTypes::WT_NONE, nullptr, nullptr); });
	menu->addAction(action);

	action = new QAction(QIcon(":/TableEngine/menu.png"), tr("&Menu"));
	connect(action, &QAction::triggered, [this](bool checked) {emit requestCommand(TeTypes::CMDID_SYSTEM_SETTING_MENU, TeTypes::WT_NONE, nullptr, nullptr); });
	menu->addAction(action);
}

/*!
 * アプリケーションの設定情報を設定ファイルから読み込む。
 */
void TeViewStore::loadSetting()
{
}

void TeViewStore::loadKeySetting()
{
	if (mp_dispatcher) mp_dispatcher->loadKeySetting();
}

/*!
 * アプリケーションの実行状態を設定ファイルから復帰させる。
 */
void TeViewStore::loadStatus()
{
	//FolderView復帰
	QSettings settings;
	QStringList paths = settings.value("initialState/paths", QStringList(QDir::rootPath())).toStringList();

	for (QString& path : paths) {
		createFolderView(path);
	}
}

void TeViewStore::show()
{
	if (mp_mainWindow) mp_mainWindow->show();
}

void TeViewStore::setDispatcher(TeDispatcher * p_dispatcher)
{
	if (mp_dispatcher) disconnect();
	mp_dispatcher = p_dispatcher;
	connect(this, &TeViewStore::requestCommand, p_dispatcher, &TeDispatcher::execCommand);
}

bool TeViewStore::dispatch(TeTypes::WidgetType type, QObject * obj, QEvent * event)
{
	if (mp_dispatcher != nullptr && mp_dispatcher->dispatch(type, obj, event)) {
		return true;
	}
	return false;
}

QWidget * TeViewStore::mainWindow()
{
	return mp_mainWindow;
}

int TeViewStore::currentTabIndex()
{
	int currentIndex = 0; 

	//カレントFolderView特定にtreeを使う
	QWidget* tree = mp_split->widget(0);

	if (tree->inherits("TeFileTreeView")) {
		for (int i = 0; i < TAB_MAX; i++) {
			if (mp_tab[i]->indexOf(qobject_cast<TeFileTreeView*>(tree)->folderView()) >= 0 ) {
				currentIndex = i;
				break;
			}
		}
	}

	return currentIndex;
}

int TeViewStore::tabIndex(TeFileFolderView * view)
{
	if(view == nullptr) return -1;
	for (int i = 0; i < TAB_MAX; i++) {
		if (mp_tab[i]->indexOf(view) >= 0) {
			return i;
		}
	}
	return -1;
}

QList<TeFileFolderView*> TeViewStore::getFolderView(int place)
{
	QList<TeFileFolderView*> list;

	if (place < TAB_MAX) {
		if (place < 0) {
			//負の場合は全Tabからかき集める。
			for (int i = 0; i < TAB_MAX; i++) {
				for (int index = 0; index < mp_tab[i]->count(); index++) {
					list.append(qobject_cast<TeFileFolderView*>(mp_tab[i]->widget(index)));
				}
			}
		}
		else {
			for (int index = 0; index < mp_tab[place]->count(); index++) {
				list.append(qobject_cast<TeFileFolderView*>(mp_tab[place]->widget(index)));
			}
		}
	}

	return list;
}

TeFileFolderView * TeViewStore::currentFolderView()
{
	return mp_currentFolderView;
}

void TeViewStore::setCurrentFolderView(TeFileFolderView * view)
{
	if (view == nullptr) return;
	//TeFileFolderView / ListViewへのフォーカス設定
	if (!view->list()->hasFocus()) {
		view->list()->setFocus();
	}

	mp_currentFolderView = view;
	int place = tabIndex(view);

	//TabWidgetのカレント変更
	int index = mp_tab[place]->indexOf(view);
	if (index >= 0) {
		if (mp_tab[place]->currentIndex() != index) {
			mp_tab[place]->setCurrentIndex(index);
		}
	}

	//TreeViewの変更 (LEFTのみTreeと連動)
	if (place == TAB_LEFT) {
		TeFileTreeView* tree = qobject_cast<TeFileTreeView*>(mp_split->widget(0));
		if (tree != view->tree()) {
			if (tree == Q_NULLPTR) {
				//新規挿入
				//QTab::addTab()から直接connect / currentChanged経由でsetCurrentFolderView()が発行されるため、
				//updateGeometory呼ばれずここにくる。最初のgeometory計算をするため明示的に呼んでおく。
				mp_tab[place]->updateGeometry();
				mp_split->insertWidget(0, view->tree());
			}
			else {
				//カレント変更
				disconnect(mp_driveBar, &TeDriveBar::changeDrive, tree->folderView(), &TeFolderView::setRootPath);
				mp_split->replaceWidget(0, view->tree());
				view->tree()->setHidden(false);
			}
			connect(mp_driveBar, &TeDriveBar::changeDrive, view, &TeFolderView::setRootPath);
		}
	}
}

TeFileFolderView * TeViewStore::createFolderView(const QString & path, int place)
{
	if (place >= TAB_MAX) place = TAB_MAX - 1;

	if (place < 0) {
		//placeが負の場合はcurrentTabに追加する。
		place = currentTabIndex();
	}
	else {
		//対象よりも若いTabがNullなら番号をずらす。
		for (int i = place; i >0; i--) {
			if (mp_tab[i - 1]->count() > 0) {
				break;
			}
			place--;
		}
	}
	QDir dir(path);
	QIcon icon = QFileIconProvider().icon(QFileInfo(path));
	TeFileFolderView * folderView = new TeFileFolderView;
	folderView->setDispatcher(this);
	folderView->setRootPath(path);
	int index = mp_tab[place]->addTab(folderView, icon, dir.isRoot() ? dir.path() : dir.dirName());
	mp_tab[place]->setTabToolTip(index, dir.absolutePath());
	mp_tab[place]->setHidden(false);

	if (currentFolderView() == nullptr) {
		//Current FolderView が登録されていない場合は登録する。
		setCurrentFolderView(folderView);
	}

	return folderView;
}

void TeViewStore::deleteFolderView(TeFileFolderView * view)
{
	if (view == nullptr) return;

	TeFileFolderView* folder = currentFolderView();
	int index = tabIndex(view);

	mp_currentFolderView = nullptr;

	delete view;

	//Tabの登録コンテンツがない場合、右tabを左に寄せる
	//Widgetは親を1つしか持てないため、別Tabに移動すると元のTabからは自動的に削除されるため
	//先頭アイテムにのみを対象してcount=0まで処理する。
	if (mp_tab[TAB_LEFT]->count() == 0) {
		int currentIndex = mp_tab[TAB_RIGHT]->currentIndex();
		int count = mp_tab[TAB_RIGHT]->count();
		while (mp_tab[TAB_RIGHT]->count()) {
			QString tip = mp_tab[TAB_RIGHT]->tabToolTip(0);
			int index = mp_tab[TAB_LEFT]->addTab(mp_tab[TAB_RIGHT]->widget(0), mp_tab[TAB_RIGHT]->tabIcon(0), mp_tab[TAB_RIGHT]->tabText(0));
			mp_tab[TAB_LEFT]->setTabToolTip(index, tip);
		}
		mp_tab[TAB_RIGHT]->clear();
		mp_tab[TAB_RIGHT]->setHidden(true);

		mp_tab[TAB_LEFT]->setCurrentIndex(currentIndex);
		if (folder != view) {
			//カレント以外の削除の場合はカレントを元にもどしておく
			setCurrentFolderView(folder);
		}
	}

	//カレントが削除された場合の補正
	if (folder == view) {
		if (mp_tab[index]->count() > 0) {
			//同一位置のタブが存在する場合
			setCurrentFolderView(qobject_cast<TeFileFolderView*>(mp_tab[index]->currentWidget()));
		}
		else {
			//タブが存在しない場合
			setCurrentFolderView(qobject_cast<TeFileFolderView*>(mp_tab[TAB_LEFT]->currentWidget()));
		}
	}
}

void TeViewStore::moveFolderView(TeFileFolderView * view, int place, int position)
{
	if (view == nullptr) return;
	if (place < 0 || TAB_MAX <= place) place = TAB_MAX-1;
	//対象よりも若いTabがNullなら番号をずらす。

	int orgPlace = tabIndex(view);
	int orgPosition = mp_tab[orgPlace]->indexOf(view);
	QString title = mp_tab[orgPlace]->tabText(orgPosition);
	QString tip = mp_tab[orgPlace]->tabToolTip(orgPosition);
	QIcon icon = mp_tab[orgPlace]->tabIcon(orgPosition);

	if ((place == orgPlace) && (position == orgPosition)) {
		//移動不要の場合は最後のカレント変更のみ実施
	} else{ 
		//新規ポジションに挿入
		if (position < 0) {
			position = mp_tab[place]->addTab(view, icon,title);
		}
		else {
			position = mp_tab[place]->insertTab(position, view, icon,title);
		}
		mp_tab[place]->setTabToolTip(position, tip);
		mp_tab[place]->setHidden(false);
	}

	//Tabの登録コンテンツがない場合、右tabを左に寄せる
	if (mp_tab[TAB_LEFT]->count() == 0) {
		int currentIndex = mp_tab[TAB_RIGHT]->currentIndex();
		int count = mp_tab[TAB_RIGHT]->count();
		//Widgetは親を1つしか持てないため、別Tabに移動すると元のTabからは自動的に削除されるため
		//先頭アイテムにのみを対象してcount=0まで処理する。
		while (mp_tab[TAB_RIGHT]->count()) {
			QString tip = mp_tab[TAB_RIGHT]->tabToolTip(0);
			int index = mp_tab[TAB_LEFT]->addTab(mp_tab[TAB_RIGHT]->widget(0), mp_tab[TAB_RIGHT]->tabIcon(0), mp_tab[TAB_RIGHT]->tabText(0));
			mp_tab[TAB_LEFT]->setTabToolTip(index, tip);
		}
		mp_tab[TAB_RIGHT]->clear();
		mp_tab[TAB_RIGHT]->setHidden(true);

		mp_tab[TAB_LEFT]->setCurrentIndex(currentIndex);
	}
	setCurrentFolderView(view);
}
