#include "TeSystemFileInfoAcsr.h"
#include "platform/platform_util.h"

TeSystemFileInfoAcsr::TeSystemFileInfoAcsr(QObject *parent)
	: TeFileInfoAcsr(parent)
{}

TeSystemFileInfoAcsr::~TeSystemFileInfoAcsr()
{}

QList<QPair<QString, TeFileInfoAcsr::ActionFunc>> TeSystemFileInfoAcsr::actions() const
{
	return QList<QPair<QString, ActionFunc>>();
}


void TeSystemFileInfoAcsr::activate(const TeFileInfo& info)
{
	openFile(info.path);
}
