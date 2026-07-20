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

#include <QDialog>
#include <QList>
#include <QSet>
#include <QString>

class QLineEdit;
class QCheckBox;
class QVBoxLayout;

/**
 * @class TeEditUserCommands
 * @brief Modal dialog for editing user-registered commands (TeTypes::CMDID_USER).
 * @ingroup dialogs
 *
 * @details Edits the variable-length list of user-registered commands stored
 * under QSettings "user/command00".."command30" (see TeSettings::SETTING_USER,
 * TeSettings::MAX_USER_COMMANDS). Each entry has an auto-assigned unique
 * 16-bit id (stable across list reordering and slot re-compaction on save),
 * a required display name, an optional icon file path (any format loadable
 * by TeAdaptiveIconEngine, which currently relies on QPixmap; stored as an
 * empty string when not set, and any load failure is left entirely to
 * TeAdaptiveIconEngine to handle), a command line (path + args, may contain
 * %F/%f/%M/%m/%P macros, see TeToolCommand), and shell/output execution
 * flags matching TeToolCommand::runCommand()'s parameters. Settings are
 * loaded from QSettings when the dialog is constructed and written back
 * only when the dialog is accepted (OK).
 *
 * @note The command-line splitting/combining helpers in this class are
 * intentionally independent from TeEditToolSetting's equivalents of the
 * same name; the two are not guaranteed to stay behaviourally identical.
 */
class TeEditUserCommands : public QDialog
{
	Q_OBJECT

	/** @brief One user-registered command entry. */
	struct UserCommandSetting {
		quint16 id     = 0;     ///< Unique id within user commands (0 = not yet assigned).
		QString name;            ///< Required display name.
		QString icon;            ///< Optional icon file path (empty = no icon registered).
		QString command;         ///< "[command path] [args]" (may contain %F/%f/%M/%m/%P macros).
		bool    shell  = false;  ///< Run via the configured shell (see TeToolCommand::runCommand).
		bool    output = false;  ///< Capture and display standard output.
	};

	/** @brief Widgets that make up one row in the command list. */
	struct UserCommandRow {
		QWidget*   frame       = nullptr; ///< Container frame for this entry.
		QLineEdit* nameEdit    = nullptr; ///< Display name input (required).
		QLineEdit* iconEdit    = nullptr; ///< Icon file path input (optional).
		QLineEdit* commandEdit = nullptr; ///< Command path input (no arguments).
		QLineEdit* argsEdit    = nullptr; ///< Arguments input (may contain %F/%f/%M/%m/%P macros).
		QCheckBox* shellCheck  = nullptr; ///< "Run via shell" checkbox.
		QCheckBox* outputCheck = nullptr; ///< "Show output" checkbox.
		quint16    id          = 0;       ///< Auto-assigned id; not user-editable, carried through save/load.
	};

public:
	TeEditUserCommands(QWidget* parent = nullptr);
	~TeEditUserCommands();

public slots:
	void accept() override;

private:
	QWidget* createUserEntry(const UserCommandSetting& initial = UserCommandSetting());
	void addUserEntry(const UserCommandSetting& initial = UserCommandSetting());
	void removeUserEntry(QWidget* frame);

	void loadSettings();
	void saveSettings();
	void applyValidityStyle(QLineEdit* edit, bool valid) const;
	bool isRowValid(const QString& path, const QString& args) const;
	quint16 allocateNewId();

	// Independent from TeEditToolSetting's helpers of the same purpose: kept
	// intentionally separate so the two dialogs' command formats can diverge.
	static void splitCommand(const QString& full, QString& path, QString& args);
	static QString combineCommand(const QString& path, const QString& args);
	static QString ensureItemMacro(const QString& args);

private:
	QVBoxLayout* mp_userLayout = nullptr; ///< Container layout holding UserCommandRow frames + trailing stretch.
	QList<UserCommandRow> m_userRows;

	QList<UserCommandSetting> m_userCommands; ///< Data model loaded on open, written back on accept().

	/**
	 * @brief Ids that must not be handed out again by allocateNewId().
	 *
	 * Seeded from QSettings at load time and grown (never shrunk) every time
	 * allocateNewId() hands out a new id, for the lifetime of this dialog
	 * instance. This means removing a row does not free its id for reuse
	 * within the same dialog session: deleting a command and then adding a
	 * new one will not silently resurrect the old id (and whatever key/menu
	 * bindings may reference it). This is a best-effort mitigation scoped to
	 * a single dialog instance, not a persisted global "never reuse" ledger.
	 */
	QSet<quint16> m_usedIds;
};
