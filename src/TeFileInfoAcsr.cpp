#include "TeFileInfoAcsr.h"

#include <QVariant>
#include "TeFileInfo.h"

TeFileInfoAcsr::TeFileInfoAcsr(QObject *parent)
	: QObject(parent)
{}

TeFileInfoAcsr::~TeFileInfoAcsr()
{}

void TeFileInfoAcsr::assignToStandardItem(QStandardItem * item)
{
	item->setData(QVariant::fromValue(this), TeFileInfo::ROLE_ACSR);
}

TeFileInfoAcsr* TeFileInfoAcsr::fromStandardItem(const QStandardItem* item)
{
	const QStandardItem* parent = item;
	TeFileInfoAcsr* acsr = parent->data(TeFileInfo::ROLE_ACSR).value<TeFileInfoAcsr*>();
	while (acsr == nullptr && parent->parent() != nullptr)
	{
		parent = parent->parent();
		acsr = parent->data(TeFileInfo::ROLE_ACSR).value<TeFileInfoAcsr*>();
	}
	return acsr;
}
