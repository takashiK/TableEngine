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
	 * @param commandTemplate Raw command string containing macros.
	 * @param p_store         Application-wide TeViewStore used to resolve current/selected items.
	 * @return The command string with all macros replaced by actual values.
	 */
	QString expandMacros(const QString& commandTemplate, TeViewStore* p_store);

	/**
	 * @brief Returns the working directory to use when executing a tool command (the %P value).
	 * @param p_store Application-wide TeViewStore.
	 * @return Absolute path of the current folder.
	 */
	QString workingDirectory(TeViewStore* p_store);

} // namespace TeToolCommand
