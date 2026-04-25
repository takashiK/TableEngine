#pragma once

#include "TeFileInfoAcsr.h"

class TeSystemFileInfoAcsr  : public TeFileInfoAcsr
{
	Q_OBJECT

public:
	TeSystemFileInfoAcsr(QObject *parent);
	~TeSystemFileInfoAcsr();

	virtual QList<QPair<QString, ActionFunc>> actions() const;

public slots:
	virtual void activate(const TeFileInfo& infos);
};
