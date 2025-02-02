#include "TeHistory.h"
#include "TeSettings.h"

TeHistory::TeHistory(QObject *parent)
	: QObject(parent)
{
	m_index = 0;
}

TeHistory::~TeHistory()
{}

void TeHistory::push(const PathPair& path)
{
	if (current() == path) {
		return;
	}
	if (m_index > 0) {
		m_history.remove(0, m_index);
		m_index = 0;
	}
	m_history.prepend(path);

	if (m_history.size() > TeSettings::MAX_HISTORY) {
		m_history.removeLast();
	}
}

TeHistory::PathPair TeHistory::pop()
{
	return m_history.takeFirst();
}

void TeHistory::clear()
{
	m_history.clear();
	m_index = 0;
}

TeHistory::PathPair TeHistory::current() const
{
	if(m_history.isEmpty())
		return PathPair();
	return m_history.at(m_index);
}

TeHistory::PathPair TeHistory::previous()
{
	if (m_index < m_history.size() - 1) {
		++m_index;
		return m_history.at(m_index);
	}
	return PathPair();
}

TeHistory::PathPair TeHistory::next()
{
	if (m_index > 0) {
		--m_index;
		return m_history.at(m_index);
	}
	return PathPair();
}

QStringList TeHistory::get() const
{
	QStringList list;
	for (const auto& pair : m_history) {
		list.append(pair.second);
	}
	return list;
}

QList<TeHistory::PathPair> TeHistory::getPair() const
{
	return m_history;
}

void TeHistory::set(const QList<PathPair>& list)
{
	m_history = list;
	m_index = 0;
}
