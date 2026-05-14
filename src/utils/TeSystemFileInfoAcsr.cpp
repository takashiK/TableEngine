#include "TeSystemFileInfoAcsr.h"
#include "platform/platform_util.h"

/**
 * @file TeSystemFileInfoAcsr.cpp
 * @brief Implementation of TeSystemFileInfoAcsr.
 * @ingroup utility
 */

/**
 * @file TeSystemFileInfoAcsr.cpp
 * @brief Implementation of TeSystemFileInfoAcsr.
 * @ingroup utility
 */

TeSystemFileInfoAcsr::TeSystemFileInfoAcsr(QObject *parent)
	: TeFileInfoAcsr(parent)
{}

TeSystemFileInfoAcsr::~TeSystemFileInfoAcsr()
{}

QList<std::pair<QString, TeFileInfoAcsr::ActionFunc>> TeSystemFileInfoAcsr::actions() const
{
	return QList<std::pair<QString, ActionFunc>>();
}


void TeSystemFileInfoAcsr::activate(const TeFileInfo& info)
{
	openFile(info.path);
}
