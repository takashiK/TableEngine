#pragma once

#include "TeFinder.h"

namespace TeArchive
{
	class Reader;
}

class TeArchiveFinder : public TeFinder
{
	Q_OBJECT

public:
	explicit TeArchiveFinder(const QString& path, QObject* parent = nullptr);
	~TeArchiveFinder();

	bool isValid() const override;

protected:
	void doSearch(const TeSearchQuery& query) override;

private:
	bool matchesEntry(const TeFileInfo& entry, const TeSearchQuery& query) const;

	TeArchive::Reader* mp_reader = nullptr;
};
