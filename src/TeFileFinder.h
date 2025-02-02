#pragma once

#include "TeFinder.h"

class TeFileFinder  : public TeFinder
{
	Q_OBJECT

public:
	TeFileFinder(const QString& path, QObject *parent=nullptr);
	~TeFileFinder();
	virtual bool isValid() const;

protected slots:
	virtual void findStep();

private:

};
