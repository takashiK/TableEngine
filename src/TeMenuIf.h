#pragma once

#include <QString>

class QMenu;
class QMenuBar;
class QAction;

class TeMenuIf {
public:
	TeMenuIf();
	TeMenuIf(const TeMenuIf& menuIf);
	TeMenuIf(QMenu* menu);
	TeMenuIf(QMenuBar* menuBar);
	virtual ~TeMenuIf();

	QMenu* addMenu(const QString& title);
	void addAction(QAction* action);
	QAction* addSeparator();

private:
	QMenu* mp_menu;
	QMenuBar* mp_menuBar;
};
