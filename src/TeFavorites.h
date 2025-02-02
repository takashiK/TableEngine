#pragma once

#include <QObject>

class TeFavorites  : public QObject
{
	Q_OBJECT

public:
	TeFavorites(QObject *parent = nullptr);
	~TeFavorites();

	void refresh();
	void save();

	bool add(const QString& path);
	void remove(const QString& path);
	void move(int index, const QString& path);
	void clear();

	QStringList get();
	void set(const QStringList& list);

private:
	QStringList m_favorites;
};
