#pragma once

#include <QObject>

/**
 * @file TeFavorites.h
 * @brief Persistent list of user-favourite directory paths.
 * @ingroup utility
 */

/**
 * @class TeFavorites
 * @brief Manages the ordered list of user-favourite directory paths.
 * @ingroup utility
 *
 * @details Provides CRUD operations on a QStringList that is persisted to
 * QSettings.  refresh() reloads from settings; save() writes back.
 * Changes made through add(), remove(), move(), and set() are kept in
 * memory until save() is called.
 */
class TeFavorites  : public QObject
{
	Q_OBJECT

public:
	TeFavorites(QObject *parent = nullptr);
	~TeFavorites();

	/** @brief Reloads the favourite list from persistent QSettings. */
	void refresh();
	/** @brief Writes the current in-memory list to persistent QSettings. */
	void save();

	/**
	 * @brief Appends @p path if not already present.
	 * @param path Absolute directory path to add.
	 * @return true if the path was added; false if already in the list.
	 */
	bool add(const QString& path);

	/**
	 * @brief Removes @p path from the list.
	 * @param path Absolute directory path to remove.
	 */
	void remove(const QString& path);

	/**
	 * @brief Moves the entry at @p index to a new position determined by @p path.
	 * @param index Current position of the entry.
	 * @param path  New absolute path for the entry.
	 */
	void move(int index, const QString& path);

	/** @brief Clears all entries without saving. */
	void clear();

	/** @brief Returns a copy of the current in-memory favourite list. */
	QStringList get();

	/**
	 * @brief Replaces the in-memory list without saving.
	 * @param list New list of absolute directory paths.
	 */
	void set(const QStringList& list);

private:
	QStringList m_favorites; ///< In-memory list of favourite paths.
};
