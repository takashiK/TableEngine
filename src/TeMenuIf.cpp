#include "TeMenuIf.h"

#include <QMenu>
#include <QMenuBar>
#include <QAction>

TeMenuIf::TeMenuIf()
{
	mp_menu = nullptr;
	mp_menuBar = nullptr;
}

TeMenuIf::TeMenuIf(const TeMenuIf& menuIf)
{
	mp_menu = menuIf.mp_menu;
	mp_menuBar = menuIf.mp_menuBar;
}

TeMenuIf::TeMenuIf(QMenu* menu)
{
	mp_menu = menu;
	mp_menuBar = nullptr;
}

TeMenuIf::TeMenuIf(QMenuBar* menuBar)
{
	mp_menu = nullptr;
	mp_menuBar = menuBar;
}

TeMenuIf::~TeMenuIf()
{
}

QMenu* TeMenuIf::addMenu(const QString& title)
{
	if (mp_menu != nullptr) {
		return mp_menu->addMenu(title);
	}
	else if (mp_menuBar != nullptr) {
		return mp_menuBar->addMenu(title);
	}

	return nullptr;
}

void TeMenuIf::addAction(QAction* action)
{
	if (mp_menu != nullptr) {
		mp_menu->addAction(action);
	}
	else if (mp_menuBar != nullptr) {
		mp_menuBar->addAction(action);
	}
}

QAction* TeMenuIf::addSeparator()
{
	if (mp_menu != nullptr) {
		return mp_menu->addSeparator();
	}
	else if (mp_menuBar != nullptr) {
		return mp_menuBar->addSeparator();
	}

	return nullptr;
}
