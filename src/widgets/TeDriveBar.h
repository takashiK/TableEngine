/****************************************************************************
**
** Copyright (C) 2021 Takashi Kuwabara.
** Contact: laffile@gmail.com
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 2 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <https://www.gnu.org/licenses/>.
**
****************************************************************************/

#pragma once

#include <QToolBar>
#include <QChar>
#include <QMap>

/**
 * @file TeDriveBar.h
 * @brief Toolbar widget showing available drives and quick-access paths.
 * @ingroup widgets
 */

class QFileSystemWatcher;

/**
 * @class TeDriveBar
 * @brief QToolBar subclass that lists local drives and user-defined quick-access
 *        paths as clickable toolbar buttons.
 * @ingroup widgets
 *
 * @details The bar is divided into two sections separated by a spacer:
 * - **Drive section** — one button per local drive letter, refreshed by
 *   updateDrive().
 * - **Quick-access section** — user-defined paths persisted via
 *   loadQuickAccesses() / storeQuickAccesses().
 *
 * Clicking either section emits driveSelected() with the corresponding path.
 */
class TeDriveBar : public QToolBar
{
	Q_OBJECT

public:
	TeDriveBar(QWidget *parent = Q_NULLPTR);
	TeDriveBar(const QString &title, QWidget *parent = Q_NULLPTR);
	virtual ~TeDriveBar();

	/**
	 * @brief Updates a single drive button identified by drive letter @p c.
	 * @param c Drive letter (e.g. 'C').
	 */
	void updateKey(const QChar& c);

	/** @brief Loads the quick-access path list from persistent settings. */
	void loadQuickAccesses();
	/** @brief Saves the quick-access path list to persistent settings. */
	void storeQuickAccesses();

	/**
	 * @brief Appends @p path to the quick-access list.
	 * @param path Absolute directory path.
	 */
	void addQuickAccess(const QString& path);

	/**
	 * @brief Removes @p path from the quick-access list.
	 * @param path Absolute directory path to remove.
	 */
	void removeQuickAccess(const QString& path);

	/**
	 * @brief Replaces the entire quick-access list with @p paths.
	 * @param paths New list of absolute directory paths.
	 */
	void setQuickAccess(const QStringList& paths);

	/** @brief Returns the current quick-access path list. */
	QStringList quickAccess() const;

	/** @brief Removes all drive and quick-access buttons from the toolbar. */
	void clear();

signals:
	/**
	 * @brief Emitted when the user clicks a drive or quick-access button.
	 * @param path The path associated with the clicked button.
	 */
	void driveSelected(const QString& path);

public slots:
	/**
	 * @brief Rebuilds the drive-button section based on currently available drives.
	 * @param state Unused; present to match QFileSystemWatcher::directoryChanged signal.
	 */
	void updateDrive(bool state);

protected:
	/**
	 * @brief Emits driveSelected() for @p path.
	 * @param path The drive or directory path to navigate to.
	 */
	void selectDrive(const QString& path);

private:
	QChar    m_key;               ///< Current drive letter (unused after init).
	QAction* mp_driveStart;       ///< Separator marking the start of the drive section.
	QAction* mp_quickStart;       ///< Separator marking the start of the quick-access section.
	QList<QAction*> m_driveActions;  ///< Per-drive toolbar actions.
	QList<QAction*> m_quickActions;  ///< Per-quick-access toolbar actions.
};
