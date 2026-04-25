#pragma once

#include "TeFinder.h"

class QFileInfo;

class TeFileFinder : public TeFinder
{
	Q_OBJECT

public:
	explicit TeFileFinder(const QString& path, QObject* parent = nullptr);
	~TeFileFinder();

	bool isValid() const override;

protected:
	void doSearch(const TeSearchQuery& query) override;

private:
	bool matchesQuery(const QFileInfo& info, const TeSearchQuery& query) const;

	QString m_rootPath;
};
