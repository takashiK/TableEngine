#pragma once

#include "TeFileInfoAcsr.h"

class TeArchiveFileInfoAcsr  : public TeFileInfoAcsr
{
	Q_OBJECT

public:
	TeArchiveFileInfoAcsr(QObject *parent);
	~TeArchiveFileInfoAcsr();

	virtual QList<QPair<QString, ActionFunc>> actions() const = 0;

public slots:
	virtual void activate(const TeFileInfo& info);
};
