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

/**
 * @file TeToolCommand.h
 * @brief Shared helpers for external tool command templates (macro expansion & validation).
 * @ingroup utility
 *
 * @details Declares the TeToolCommand namespace used by both TeToolDialog
 * (command format validation while editing) and command implementations such
 * as TeCmdRunCommand / TeCmdToolFileEdit / TeCmdToolBinaryEdit (macro
 * expansion & working directory resolution at execution time).
 *
 * Supported macros within a command template string:
 * - `%F` : current item path (TeUtils::getCurrentItem()).
 * - `%f` : current item file name only (file name part of %F).
 * - `%M` : selected item paths, space separated (TeUtils::getSelectedItemList()).
 * - `%m` : selected item file names only, space separated (file name parts of %M).
 * - `%P` : current folder path (TeUtils::getCurrentFolder()); also used as the
 *          working directory when executing the command.
 *
 * Any substituted value containing a space is wrapped in double quotes so
 * that QProcess::splitCommand() tokenises the expanded string correctly.
 */

#include <QString>

class TeViewStore;

/**
 * @namespace TeToolCommand
 * @brief Macro expansion and validation helpers for tool command templates.
 */
namespace TeToolCommand {

	/**
	 * @brief Checks whether a (not yet expanded) command template is syntactically usable.
	 *
	 * Performs a lightweight structural check using QProcess::splitCommand():
	 * the template must yield at least one non-empty token (the command path).
	 * An empty/blank template is considered valid (meaning "not configured").
	 *
	 * @param commandTemplate Raw command string, possibly containing %F/%f/%M/%m/%P macros.
	 * @return true when the template is empty or has a resolvable command token.
	 */
	bool isValidFormat(const QString& commandTemplate);

	/**
	 * @brief Expands %F/%f/%M/%m/%P macros in @p commandTemplate using the current view state.
	 *
	 * Directories are excluded from %F/%f (the single current item) and %M/%m
	 * (the selected item list) — see getSelectedFileList(). When the current
	 * item is a directory, %F/%f expand to an empty string.
	 *
	 * @param commandTemplate Raw command string containing macros.
	 * @param p_store         Application-wide TeViewStore used to resolve current/selected items.
	 * @param p_hasTarget     Optional out-parameter. Set to false when the template references
	 *                        %F/%f and/or %M/%m but no target file exists (current item is a
	 *                        directory / nothing selected, or no non-directory items are
	 *                        selected). Set to true otherwise (including when none of these
	 *                        macros are used).
	 * @return The command string with all macros replaced by actual values.
	 */
	QString expandMacros(const QString& commandTemplate, TeViewStore* p_store, bool* p_hasTarget = nullptr);

	/**
	 * @brief Returns the working directory to use when executing a tool command (the %P value).
	 * @param p_store Application-wide TeViewStore.
	 * @return Absolute path of the current folder.
	 */
	QString workingDirectory(TeViewStore* p_store);

	/**
	 * @brief Expands and executes a command template via QProcess.
	 *
	 * Shared by TeCmdRunCommand, TeCmdToolFileEdit and TeCmdToolBinaryEdit. Does
	 * nothing when @p commandTemplate is empty, or when it references
	 * %F/%f/%M/%m but no target file exists (see expandMacros()).
	 *
	 * When @p shell is true, resolves SETTING_COMMAND_Shell / SETTING_COMMAND_ShellArg
	 * from QSettings; if no shell command is configured, falls back to direct
	 * execution (same as shell == false). When @p output is true, captures
	 * standard output and displays it in a floating QPlainTextEdit registered
	 * on @p p_store.
	 *
	 * @param p_store         Application-wide TeViewStore.
	 * @param commandTemplate Raw command string (may contain %F/%f/%M/%m/%P macros).
	 * @param shell           Whether to run the command through the configured shell.
	 * @param output          Whether to capture and display standard output.
	 */
	void runCommand(TeViewStore* p_store, const QString& commandTemplate, bool shell, bool output);

	/**
	 * @brief Finds a user-defined tool command (tools/user/tool01..MAX_USER_TOOLS) matching
	 *        the file's suffix.
	 * @param path File path used to extract the suffix (extension) for matching (case-insensitive).
	 * @return The raw command template if a match is found; otherwise an empty string.
	 */
	QString findUserToolCommand(const QString& path);

} // namespace TeToolCommand
