#pragma once

#include <QObject>
#include <QList>
#include <QAction>
#include <QStandardItem>

#include "TeFileInfo.h"
#include "TeDispatchable.h"

class TeFileInfoAcsr  : public QObject
{
	Q_OBJECT
public:
	typedef void(*ActionFunc)(TeDispatchable* dispatcher, TeTypes::WidgetType type, const QList<TeFileInfo>& infos);

public:
	TeFileInfoAcsr(QObject *parent);
	virtual ~TeFileInfoAcsr();

	// return a list of actions that can be performed on the given file
	// QSting is the name of the action, ActionFunc is the function to call
	virtual QList<QPair<QString, ActionFunc>> actions() const = 0;

	// assing the item to this accessor.
	// and this accessor owned by the item.
	void assignToStandardItem( QStandardItem* item);
	static TeFileInfoAcsr* fromStandardItem(const QStandardItem* item);

public slots:
	virtual void activate(const TeFileInfo& info) = 0;
};
