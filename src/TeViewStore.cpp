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

#include "TeViewStore.h"
#include "widgets/TeMainWindow.h"
#include "widgets/TeFileListView.h"
#include "widgets/TeFileTreeView.h"
#include "widgets/TeFileFolderView.h"
#include "widgets/TeArchiveFolderView.h"
#include "widgets/TeDriveBar.h"
#include "TeDispatcher.h"
#include "TeSettings.h"
#include "commands/TeCommandFactory.h"
#include "commands/TeCommandInfo.h"
#include "commands/folder/TeCmdFolderChangeRoot.h"
#include "TeEventEmitter.h"
#include "TeUtils.h"
#include "dialogs/TeFilePathDialog.h"

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
#include <QStack>
#include <QDebug>

TeViewStore::TeViewStore(QObject *parent)
	: QObject(parent)
{
	m_currentTabPlace = TAB_LEFT;
	mp_dispatcher = nullptr;
	mp_driveBar = nullptr;
	mp_mainWindow = nullptr;
	mp_tab[TAB_LEFT] = nullptr;
	mp_tab[TAB_RIGHT] = nullptr;
	mp_split = nullptr;
	mp_closeEventEmitter = nullptr;
	mp_focusEventEmitter = nullptr;
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

	for (auto widget : m_floatingWidgets) {
		delete widget;
	}
	m_floatingWidgets.clear();
	delete mp_closeEventEmitter;
}

TeTypes::WidgetType TeViewStore::getType() const
{
	return TeTypes::WT_NONE;
}

void TeViewStore::initialize()
{
	//Main window
	mp_mainWindow = new TeMainWindow;

	//Drive bar
	mp_driveBar = new TeDriveBar("Drive Bar");
	mp_driveBar->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(mp_driveBar, &TeDriveBar::customContextMenuRequested, [this](const QPoint& pos) {
		QAction* act = mp_driveBar->actionAt(pos);
		QString path;
		if (act != nullptr) {
			path = act->data().toString();
		}
		QMenu menu;
		QAction* act1 = menu.addAction(tr("Add Quick Access"));
		connect(act1, &QAction::triggered, [this, path](bool) {
			TeFilePathDialog dlg;
			dlg.setCurrentPath(getCurrentFolder(this));
			dlg.setTargetPath(getCurrentFolder(this));
			dlg.setFavorites(getFavorites());
			dlg.setHistory(currentFolderView()->getPathHistory());
			if (dlg.exec() == QDialog::Accepted) {
				QFileInfo info(dlg.targetPath());
				if (info.exists() && info.isDir()) {
					mp_driveBar->addQuickAccess(info.absoluteFilePath());
					mp_driveBar->storeQuickAccesses();
				}
			}

			});
		QAction* act2 = menu.addAction(tr("Remove Quick Access"));
		if (act == nullptr) {
			act2->setEnabled(false);
		}
		connect(act2, &QAction::triggered, [this, path](bool) {
			mp_driveBar->removeQuickAccess(path);
			mp_driveBar->storeQuickAccesses();
			});
		menu.exec(mp_driveBar->mapToGlobal(pos));
		});

	connect(mp_driveBar, &TeDriveBar::driveSelected, [this](const QString& path) {
		TeCmdParam param;
		param.insert(TeCmdFolderChangeRoot::PARAM_ROOT_PATH, path);
		emit requestCommand(TeTypes::CMDID_SYSTEM_FOLDER_CHANGE_ROOT,TeTypes::WT_DRIVEBAR,nullptr,&param);
		});
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
	mp_tab[TAB_LEFT]->setTabsClosable(true);

	mp_tab[TAB_RIGHT] = new QTabWidget();
	mp_tab[TAB_RIGHT]->setMovable(true);
	mp_tab[TAB_RIGHT]->setHidden(true);
	mp_tab[TAB_RIGHT]->setTabsClosable(true);

	QHBoxLayout *hbox = new QHBoxLayout();
	hbox->setContentsMargins(0, 0, 0, 0);
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

	connect(mp_tab[TAB_LEFT], &QTabWidget::currentChanged, [this](int index) {setCurrentFolderView(qobject_cast<TeFolderView*>(mp_tab[TAB_LEFT]->widget(index))); });
	connect(mp_tab[TAB_RIGHT], &QTabWidget::currentChanged, [this](int index) {setCurrentFolderView(qobject_cast<TeFolderView*>(mp_tab[TAB_RIGHT]->widget(index))); });

	connect(mp_tab[TAB_LEFT], &QTabWidget::tabCloseRequested, [this](int index) { deleteFolderView(qobject_cast<TeFolderView*>(mp_tab[TAB_LEFT]->widget(index)) ); });
	connect(mp_tab[TAB_RIGHT], &QTabWidget::tabCloseRequested, [this](int index) { deleteFolderView(qobject_cast<TeFolderView*>(mp_tab[TAB_RIGHT]->widget(index)) ); });

	//setup command bridge
	connect(this, &TeViewStore::requestCommand,
		[this](TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param) { execCommand(cmdId, type, event, p_param); });

	//setup closeEventEmitter
	mp_closeEventEmitter = new TeEventEmitter();
	mp_closeEventEmitter->addEventType(QEvent::Close);
	connect(mp_closeEventEmitter, &TeEventEmitter::emitEvent, this, &TeViewStore::floatingWidgetClosed, Qt::QueuedConnection);

	//setup focusEventEmitter
	mp_focusEventEmitter = new TeEventEmitter();
	mp_focusEventEmitter->addEventType(QEvent::FocusIn);
	connect(mp_focusEventEmitter, &TeEventEmitter::emitEvent, this, &TeViewStore::focusFolderViewChanged, Qt::QueuedConnection);

	//load settings
	loadMenu();
	loadSetting();
	loadStatus();
}

/*!
 * Store menu item from settings.
 */
void TeViewStore::loadMenu()
{
	// clear menu
	mp_mainWindow->menuBar()->clear();

	// update menu
	QSettings settings;
	TeCommandFactory* p_factory = TeCommandFactory::factory();

	settings.beginGroup(SETTING_MENU);
	settings.beginGroup(SETTING_MENUBAR_GROUP);
	QStack<QMenu*> menus;
	for (const auto& key : settings.childKeys()) {
		QStringList values = settings.value(key).toString().split(',');
		int indent = values[0].toInt();
		TeTypes::CmdId cmdId = static_cast<TeTypes::CmdId>(values[2].toInt());
		QString name = values[1];

		if (indent < 0) {
			qDebug() << "Setting File Error: Invalid menu indent.";
			continue;
		}

		while (indent < menus.count()) {
			menus.pop();
		}

		if (cmdId == TeTypes::CMDID_SPECIAL_FOLDER) {
			//Create Menu Folder
			if (menus.isEmpty()) {
				//Create Top menu folder
				menus.push( mp_mainWindow->menuBar()->addMenu(name) );
			}
			else {
				//Create Sub menu folder
				menus.push( menus.top()->addMenu(name) );
			}
		}
		else if (cmdId == TeTypes::CMDID_SPECIAL_SEPARATOR) {
			//add Separator
			if (menus.isEmpty()) {
				qDebug() << "Setting File Error: Menu item can't append top level menu.";
			}
			else {
				menus.top()->addSeparator();
			}
		}
		else {
			//Create Menu Item
			if (menus.isEmpty()) {
				qDebug() << "Setting File Error: Menu item can't append top level menu.";
			}
			else {
				const TeCommandInfoBase* p_info = p_factory->commandInfo(cmdId);
				if (p_info) {
					QAction* action = new QAction(p_info->icon(), p_info->name());
					connect(action, &QAction::triggered, [this, cmdId](bool /*checked*/) { emit requestCommand(cmdId, TeTypes::WT_NONE, nullptr, nullptr); });
					menus.top()->addAction(action);
				}
			}
		}
	}
	settings.endGroup();
	settings.endGroup();

	QList<QPair<QString, TeTypes::CmdId>>  list = TeCommandFactory::static_groupList();


	for (const auto& groupItem : list) {
		QMenu* menu = mp_mainWindow->menuBar()->addMenu(groupItem.first);
		for (const auto& item : p_factory->commandGroup(groupItem.second)) {
			QAction* action = new QAction(item->icon(), item->name());
			connect(action, &QAction::triggered, [this, item](bool /*checked*/) { emit requestCommand(item->cmdId(), TeTypes::WT_NONE, nullptr, nullptr); });
			menu->addAction(action);
		}
	}
}

/*!
	Store Application settings
 */
void TeViewStore::loadSetting()
{
}

void TeViewStore::loadKeySetting()
{
	if (mp_dispatcher) mp_dispatcher->loadKeySetting();
}

/*!
	Store application status.
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

void TeViewStore::close()
{
	if (mp_mainWindow) mp_mainWindow->close();
}

void TeViewStore::setDispatcher(TeDispatcher * p_dispatcher)
{
	mp_dispatcher = p_dispatcher;
}

bool TeViewStore::dispatch(TeTypes::WidgetType type, QEvent * event)
{
	if (mp_dispatcher != nullptr && mp_dispatcher->dispatch(type, event)) {
		return true;
	}
	return false;
}

void TeViewStore::execCommand(TeTypes::CmdId cmdId, TeTypes::WidgetType type, QEvent* event, const TeCmdParam* p_param)
{
	if (mp_dispatcher != nullptr) {
		mp_dispatcher->execCommand(cmdId, type, event,p_param);
	}
}

QWidget * TeViewStore::mainWindow()
{
	return mp_mainWindow;
}

int TeViewStore::currentTabPlace()
{
	return m_currentTabPlace;
}

int TeViewStore::currentTabIndex()
{
	int currentIndex = 0; 

	//If FolderView is current then treeview that child of FolderView is set to left part of split.
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

int TeViewStore::tabPlace(TeFolderView * view)
{
	if(view == nullptr) return -1;
	for (int i = 0; i < TAB_MAX; i++) {
		if (mp_tab[i]->indexOf(view) >= 0) {
			return i;
		}
	}
	return -1;
}

TeFolderView* TeViewStore::getFolderView(int place)
{
	TeFolderView* p_folder = nullptr;
	if (place >= 0 && place < TAB_MAX) {
		p_folder = qobject_cast<TeFolderView*>(mp_tab[place]->currentWidget());
	}

	return p_folder;
}

/**
 * @brief TeViewStore::getFolderViews
 * @param place if place < 0 then return all folder views. else return folder views of place.
 */
QList<TeFolderView*> TeViewStore::getFolderViews(int place)
{
	QList<TeFolderView*> list;

	if (place < TAB_MAX) {
		if (place < 0) {
			for (int i = 0; i < TAB_MAX; i++) {
				for (int index = 0; index < mp_tab[i]->count(); index++) {
					list.append(qobject_cast<TeFolderView*>(mp_tab[i]->widget(index)));
				}
			}
		}
		else {
			for (int index = 0; index < mp_tab[place]->count(); index++) {
				list.append(qobject_cast<TeFolderView*>(mp_tab[place]->widget(index)));
			}
		}
	}

	return list;
}

TeFolderView * TeViewStore::currentFolderView()
{
	return getFolderView(currentTabPlace());
}

void TeViewStore::setCurrentFolderView(TeFolderView * view)
{
	if (view == nullptr) return;

	//Focus setting to TeFolderView / ListView
	if (!view->list()->hasFocus()) {
		view->list()->setFocus();
	}

	int place = tabPlace(view);
	m_currentTabPlace = place;

	//Change current of TabWidget
	int index = mp_tab[place]->indexOf(view);
	if (index >= 0) {
		if (mp_tab[place]->currentIndex() != index) {
			mp_tab[place]->setCurrentIndex(index);
		}
	}

	//Change TreeView if it is member of TAB_LEFT.
	if (place == TAB_LEFT) {
		TeFileTreeView* tree = qobject_cast<TeFileTreeView*>(mp_split->widget(0));
		if (tree != view->tree()) {
			if (tree == Q_NULLPTR) {
				//Insert New Item
				//In this section directry excute after call QTab::addTab(). because it emit currentChanged and currentChanged call setCurrentFolderView().
				//So we need call updategeometory() before call any geometoric function.
				mp_tab[place]->updateGeometry();
				mp_split->insertWidget(0, view->tree());
			}
			else {
				//change current
				mp_split->replaceWidget(0, view->tree());
				view->tree()->setHidden(false);
			}
			tree = view->tree();
		}
	}
}

TeFileFolderView * TeViewStore::createFolderView(const QString & path, int place)
{
	TeFileFolderView * folderView = new TeFileFolderView;
	folderView->setDispatcher(this);
	folderView->setRootPath(path);
	folderView->list()->setFocusPolicy(Qt::ClickFocus);
	folderView->tree()->setFocusPolicy(Qt::ClickFocus);

	addFolderView(folderView, place);

	return folderView;
}

TeArchiveFolderView* TeViewStore::createArchiveFolderView(const QString& path, int place)
{
	TeArchiveFolderView* folderView = new TeArchiveFolderView;
	folderView->setDispatcher(this);
	folderView->setArchive(path);
	folderView->list()->setFocusPolicy(Qt::ClickFocus);
	folderView->tree()->setFocusPolicy(Qt::ClickFocus);

	addFolderView(folderView, place);

	return folderView;
}

void TeViewStore::deleteFolderView(TeFolderView * view)
{
	if (view == nullptr) return;

	TeFolderView* folder = currentFolderView();
	int index = tabPlace(view);

	bool isCurrentDelete = (folder == view);
	delete view;

	//If Left Tab are empty then Right Tab entries move to Left Tab.
	if (mp_tab[TAB_LEFT]->count() == 0) {
		int currentIndex = mp_tab[TAB_RIGHT]->currentIndex();
		while (mp_tab[TAB_RIGHT]->count()) {
			//Widget has only one parent. so widget that child of Right tab set to Left. that mean is widget move Right to Left.
			//after all of Right tab children set to Left then Right tab's children are vanished. so this while loop can exist.
			QString tip = mp_tab[TAB_RIGHT]->tabToolTip(0);
			int index = mp_tab[TAB_LEFT]->addTab(mp_tab[TAB_RIGHT]->widget(0), mp_tab[TAB_RIGHT]->tabIcon(0), mp_tab[TAB_RIGHT]->tabText(0));
			mp_tab[TAB_LEFT]->setTabToolTip(index, tip);
		}
		mp_tab[TAB_RIGHT]->clear();
		mp_tab[TAB_RIGHT]->setHidden(true);

		mp_tab[TAB_LEFT]->setCurrentIndex(currentIndex);
		if (!isCurrentDelete) {
			//repair current.
			setCurrentFolderView(folder);
		}
	}

	if (mp_tab[TAB_RIGHT]->count() == 0)
		mp_tab[TAB_RIGHT]->setHidden(true);

	if (!mp_tab[TAB_RIGHT]->isHidden())
		mp_tab[TAB_LEFT]->setTabBarAutoHide(false);

	//target new current folder if current folder is deleted.
	if (isCurrentDelete) {
		if (mp_tab[index]->count() > 0) {
			setCurrentFolderView(qobject_cast<TeFolderView*>(mp_tab[index]->currentWidget()));
		}
		else {
			setCurrentFolderView(qobject_cast<TeFolderView*>(mp_tab[TAB_LEFT]->currentWidget()));
		}
	}
}

void TeViewStore::moveFolderView(TeFolderView * view, int place, int index)
{
	if (view == nullptr) return;
	if (place < 0 || TAB_MAX <= place) place = TAB_MAX-1;

	int orgPlace = tabPlace(view);
	int orgIndex = mp_tab[orgPlace]->indexOf(view);
	QString title = mp_tab[orgPlace]->tabText(orgIndex);
	QString tip = mp_tab[orgPlace]->tabToolTip(orgIndex);
	QIcon icon = mp_tab[orgPlace]->tabIcon(orgIndex);

	if ((place == orgPlace) && (index == orgIndex)) {
		//don't need move.
	} else{ 
		//insert to new index.
		if (index < 0) {
			index = mp_tab[place]->addTab(view, icon,title);
		}
		else {
			index = mp_tab[place]->insertTab(index, view, icon,title);
		}
		mp_tab[place]->setTabToolTip(index, tip);
		mp_tab[place]->setHidden(false);
	}

	//Correct Tab index If Left Tab entry is vanished.
	if (mp_tab[TAB_LEFT]->count() == 0) {
		int currentIndex = mp_tab[TAB_RIGHT]->currentIndex();
		while (mp_tab[TAB_RIGHT]->count()) {
			QString tip = mp_tab[TAB_RIGHT]->tabToolTip(0);
			int index = mp_tab[TAB_LEFT]->addTab(mp_tab[TAB_RIGHT]->widget(0), mp_tab[TAB_RIGHT]->tabIcon(0), mp_tab[TAB_RIGHT]->tabText(0));
			mp_tab[TAB_LEFT]->setTabToolTip(index, tip);
		}
		mp_tab[TAB_RIGHT]->clear();
		mp_tab[TAB_RIGHT]->setHidden(true);

		mp_tab[TAB_LEFT]->setCurrentIndex(currentIndex);
	}

	if (mp_tab[TAB_RIGHT]->count() == 0)
		mp_tab[TAB_RIGHT]->setHidden(true);

	if (!mp_tab[TAB_RIGHT]->isHidden())
		mp_tab[TAB_LEFT]->setTabBarAutoHide(false);

	setCurrentFolderView(view);
}

void TeViewStore::registerFloatingWidget(QWidget* widget)
{
	if (!m_floatingWidgets.contains(widget)) {
		m_floatingWidgets.append(widget);
		mp_closeEventEmitter->addOneShotEmiter(widget);
	}
}

void TeViewStore::changeRootPath(const QString& path, bool newView, int place)
{
	if (place >= TAB_MAX) return;

	if (place < 1) {
		place = currentTabIndex();
	}

	TeFolderView* view = getFolderView(place);
	if (!view || view->getType() != TeTypes::WT_FOLDERVIEW) {
		newView = true;
	}

	if (!newView && view && (view->getType() == TeTypes::WT_FOLDERVIEW)) {
		int index = mp_tab[place]->currentIndex();
		QDir dir(path);
		QIcon icon = QFileIconProvider().icon(QFileInfo(path));

		mp_tab[place]->setTabToolTip(index, path);
		mp_tab[place]->setTabText(index, dir.isRoot() ? dir.path() : dir.dirName());
		mp_tab[place]->setTabIcon(index, icon);
		view->setRootPath(path);
	}
	else {
		createFolderView(path, place);
	}
}

void TeViewStore::focusFolderViewChanged(QWidget* widget, QEvent* event)
{
	NOT_USED(event);
	TeFolderView* p_folder = qobject_cast<TeFolderView*>(widget);
	if (p_folder) {
		int place = tabPlace(p_folder);
		if (place != m_currentTabPlace) {
			setCurrentFolderView(p_folder);
		}
	}
}

void TeViewStore::addFolderView(TeFolderView* folderView, int place)
{
	if (place >= TAB_MAX) place = TAB_MAX - 1;

	if (place < 0) {
		place = currentTabPlace();
	}
	else {
		//correct null tab.
		for (int i = place; i > 0; i--) {
			if (mp_tab[i - 1]->count() > 0) {
				break;
			}
			place--;
		}
	}
	QDir dir(folderView->rootPath());
	QIcon icon = QFileIconProvider().icon(QFileInfo(folderView->rootPath()));
	int index = mp_tab[place]->addTab(folderView, icon, dir.isRoot() ? dir.path() : dir.dirName());
	mp_tab[place]->setTabToolTip(index, dir.absolutePath());
	mp_tab[place]->setHidden(false);

	if(!mp_tab[TAB_RIGHT]->isHidden())
		mp_tab[TAB_LEFT]->setTabBarAutoHide(false);

	setCurrentFolderView(folderView);
}

void TeViewStore::floatingWidgetClosed(QWidget* widget, QEvent* )
{
	if (m_floatingWidgets.contains(widget)) {
		m_floatingWidgets.removeAll(widget);
		delete widget;
	}
}
