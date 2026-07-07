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
#include <QString>
#include <QStringList>

class QLineEdit;
class QVBoxLayout;

/**
 * @class TeEditToolSetting
 * @brief Modal dialog for editing external tool settings.
 * @ingroup dialogs
 *
 * @details Edits the fixed text/image/binary editor commands (General tab)
 * and the variable-length list of user-defined tools (User tab). Settings
 * are loaded from QSettings when the dialog is constructed and written back
 * only when the dialog is accepted (OK).
 */
class TeEditToolSetting : public QDialog
{
	Q_OBJECT

	/** @brief One user-defined tool entry: command line + associated suffixes. */
	struct ToolSetting {
		QString command;      ///< "[command path] [args]" (may contain %F/%f/%M/%m/%P macros).
		QStringList suffixes;  ///< File suffixes without a leading '.' (e.g. "tar.gz").
	};

	/** @brief Widgets that make up one row in the User tab. */
	struct UserRow {
		QWidget*   frame       = nullptr; ///< Container frame for this entry.
		QLineEdit* commandEdit = nullptr; ///< Command path input (no arguments).
		QLineEdit* argsEdit    = nullptr; ///< Arguments input (may contain %F/%f/%M/%m/%P macros).
		QLineEdit* suffixEdit  = nullptr; ///< "suffix1; suffix2; suffix3" input.
	};

public:
	TeEditToolSetting(QWidget* parent = nullptr);
	~TeEditToolSetting();
	static void storeDefaultSettings();

public slots:
	void accept() override;

private:
	QWidget* createPageGeneral();
	QWidget* createPageUser();
	QWidget* createUserEntry(const ToolSetting& initial = ToolSetting());
	void addUserEntry(const ToolSetting& initial = ToolSetting());
	void removeUserEntry(QWidget* frame);

	void loadSettings();
	void saveSettings();
	void applyValidityStyle(QLineEdit* edit, bool valid) const;
	bool isRowValid(const QString& path, const QString& args) const;

	static void splitCommand(const QString& full, QString& path, QString& args);
	static QString combineCommand(const QString& path, const QString& args);
	static QString ensureItemMacro(const QString& args);

private:
	// General tab widgets
	QLineEdit* mp_textEdit       = nullptr;
	QLineEdit* mp_textArgsEdit   = nullptr;
	QLineEdit* mp_imageEdit      = nullptr;
	QLineEdit* mp_imageArgsEdit  = nullptr;
	QLineEdit* mp_binaryEdit     = nullptr;
	QLineEdit* mp_binaryArgsEdit = nullptr;

	// User tab widgets
	QVBoxLayout* mp_userLayout = nullptr; ///< Container layout holding UserRow frames + trailing stretch.
	QList<UserRow> m_userRows;

	// Data model (loaded on open, written back on accept())
	QString m_textTool;
	QString m_binaryTool;
	QString m_imageTool;

	QList<ToolSetting> m_userTools;
};
