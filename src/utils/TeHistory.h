#pragma once

#include <QObject>
#include <QList>
#include <QPair>

class TeHistory  : public QObject
{
	Q_OBJECT

public:
	typedef QPair<QString, QString> PathPair;

	TeHistory(QObject *parent = nullptr);
	~TeHistory();

	void push(const PathPair& path);
	QPair<QString,QString> pop();
	void clear();

	PathPair current() const;
	PathPair previous();
	PathPair next();

	QStringList get() const;
	QList<PathPair> getPair() const;
	void set(const QList<PathPair>& list);

private:
	int m_index;
	QList<PathPair> m_history;
};
