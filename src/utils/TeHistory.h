#pragma once

#include <QObject>
#include <QList>
#include <QPair>

/**
 * @file TeHistory.h
 * @brief Navigation history for folder-view path pairs.
 * @ingroup utility
 */

/**
 * @class TeHistory
 * @brief Maintains a double-ended navigation history of (root, current) path
 *        pairs for TeFolderView.
 * @ingroup utility
 *
 * @details The history is modelled as a linear list with a cursor index.
 * push() adds a new entry and advances the cursor.  previous() / next() move
 * the cursor without adding entries.  The list is truncated on push() if
 * there are "future" entries (i.e. the user had navigated back and then
 * chose a new path).
 */
class TeHistory  : public QObject
{
	Q_OBJECT

public:
	typedef QPair<QString, QString> PathPair; ///< (root path, current path) pair.

	TeHistory(QObject *parent = nullptr);
	~TeHistory();

	/**
	 * @brief Pushes a new path pair onto the history stack.
	 *
	 * Any entries ahead of the current cursor are discarded.
	 * @param path The (root, current) pair to record.
	 */
	void push(const PathPair& path);

	/**
	 * @brief Removes and returns the current entry.
	 * @return The removed PathPair.
	 */
	QPair<QString,QString> pop();

	/** @brief Clears all history entries. */
	void clear();

	/** @brief Returns the entry at the current cursor without moving it. */
	PathPair current() const;

	/**
	 * @brief Moves the cursor back one step and returns the entry.
	 * @return The previous PathPair, or an empty pair if at the beginning.
	 */
	PathPair previous();

	/**
	 * @brief Moves the cursor forward one step and returns the entry.
	 * @return The next PathPair, or an empty pair if at the end.
	 */
	PathPair next();

	/** @brief Returns all current-path strings in history order. */
	QStringList get() const;
	/** @brief Returns all (root, current) pairs in history order. */
	QList<PathPair> getPair() const;

	/**
	 * @brief Replaces the history list with @p list and resets the cursor.
	 * @param list New history to install.
	 */
	void set(const QList<PathPair>& list);

private:
	int             m_index;   ///< Current cursor position in m_history.
	QList<PathPair> m_history; ///< All recorded path pairs.
};
