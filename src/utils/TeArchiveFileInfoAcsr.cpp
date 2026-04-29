#include "TeArchiveFileInfoAcsr.h"

/**
 * @file TeArchiveFileInfoAcsr.cpp
 * @brief Implementation of TeArchiveFileInfoAcsr.
 * @ingroup utility
 */

/**
 * @file TeArchiveFileInfoAcsr.cpp
 * @brief Implementation of TeArchiveFileInfoAcsr.
 * @ingroup utility
 */

TeArchiveFileInfoAcsr::TeArchiveFileInfoAcsr(QObject *parent)
	: TeFileInfoAcsr(parent)
{}

TeArchiveFileInfoAcsr::~TeArchiveFileInfoAcsr()
{}

QList<QPair<QString, TeFileInfoAcsr::ActionFunc>> TeArchiveFileInfoAcsr::actions() const
{
	return QList<QPair<QString, ActionFunc>>();
}

void TeArchiveFileInfoAcsr::activate(const TeFileInfo& info)
{
}
