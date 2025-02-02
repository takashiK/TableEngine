#include "TeFavorites.h"

#include "TeSettings.h"
#include <QSettings>

TeFavorites::TeFavorites(QObject *parent)
	: QObject(parent)
{
	refresh();
}

TeFavorites::~TeFavorites()
{
}

void TeFavorites::refresh()
{
	m_favorites.clear();
	QSettings settings;
	settings.beginGroup(SETTING_FAVORITES);
	for (const auto& key : settings.childKeys()) {
		m_favorites.append(settings.value(key).toString());
	}
	settings.endGroup();
}

void TeFavorites::save()
{
	if (m_favorites.size() > TeSettings::MAX_FAVORITES) {
		m_favorites.remove(TeSettings::MAX_FAVORITES,m_favorites.size()- TeSettings::MAX_FAVORITES);
	}

	QSettings settings;
	settings.beginGroup(SETTING_FAVORITES);
	settings.remove("");
	for (int i = 0; i < m_favorites.size(); ++i) {
		settings.setValue(QString("path%1").arg(i, 2, 10, QChar('0')), m_favorites.at(i));
	}
	settings.endGroup();
}

bool TeFavorites::add(const QString & path)
{
	if (m_favorites.size() > TeSettings::MAX_FAVORITES) {
		return false;
	}
	m_favorites.removeAll(path);
	m_favorites.prepend(path);
	return true;
}

void TeFavorites::remove(const QString& path)
{
	m_favorites.removeAll(path);
}

void TeFavorites::move(int index, const QString& path)
{
	m_favorites.removeAll(path);
	m_favorites.insert(index, path);
}

void TeFavorites::clear()
{
	m_favorites.clear();
}

QStringList TeFavorites::get()
{
	return m_favorites;
}

void TeFavorites::set(const QStringList& list)
{
	m_favorites = list;
}
