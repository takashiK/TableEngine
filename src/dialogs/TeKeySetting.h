#pragma once

#include "TeTypes.h"

#include <QDialog>
#include <QList>

class QTreeWidget;
class QTreeWidgetItem;

class TeKeySetting : public QDialog
{
	Q_OBJECT

public:
	TeKeySetting(QWidget *parent);
	virtual ~TeKeySetting();

	void updateSettings();
	static void storeDefaultSettings();

public slots:
	void accept();

protected:
	QList<QTreeWidgetItem*> createKeyTreeItem();
	QList<QTreeWidgetItem*> createKeyItem( const QList<int>& list);
	QList<QTreeWidgetItem*> createCmdTreeItem();
	QList<QTreeWidgetItem*> createCmdItem( TeTypes::CmdId cmdGroup );

private:
	QTreeWidget* mp_list;
};
