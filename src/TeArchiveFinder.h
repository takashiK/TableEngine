#pragma once

#include "TeFinder.h"

namespace TeArchive
{
	class Reader;
}

class TeArchiveFinder  : public TeFinder
{
	Q_OBJECT

public:
	TeArchiveFinder(const QString& path, QObject *parent=nullptr);
	~TeArchiveFinder();
	virtual bool isValid() const;

protected slots:
	virtual void findStep();

private:
	TeArchive::Reader* mp_reader;
};
