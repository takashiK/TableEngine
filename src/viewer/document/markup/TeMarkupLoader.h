#pragma once
#include <QString>
#include <QMap>

class TeMarkupLoader
{
public:
	TeMarkupLoader();
	virtual ~TeMarkupLoader();
	
	bool loadAll();
	bool saveAll();

	bool setDefaultContainer(const QString& path, bool overwrite=false);
	int addRelation(const QString& suffix, const QString& path, bool overwrite=false);
	int delRelation(const QString& suffix);

	const QString relatedContainer(const QString& suffix);

private:
	QString m_defaultContainer;
	QMap<QString, QString> m_relations;
};

