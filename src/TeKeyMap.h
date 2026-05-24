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

#include "TeTypes.h"

#include <QHash>
#include <QList>
#include <utility>

class QKeyEvent;

/**
 * @file TeKeyMap.h
 * @brief Single source of truth for the set of assignable key bindings.
 * @ingroup main
 */

/**
 * @class TeKeyMap
 * @brief Provides the canonical list of key combos that can be bound to commands.
 * @ingroup main
 *
 * @details Centralises the key-set that was previously duplicated across
 * TeDispatcher::loadKeySetting(), TeKeySetting::createKeyTreeItem(), and
 * TeFolderView::isDispatchable().
 */
class TeKeyMap
{
public:
	/**
	 * @brief Returns every int-encoded key combination that may be bound to a command.
	 *
	 * Each entry encodes modifier flags (Qt::CTRL, Qt::SHIFT, …) OR-ed with a
	 * plain Qt::Key_* value, exactly matching the encoding used by QKeyEvent and
	 * QKeySequence.
	 *
	 * Groups covered:
	 *  - A–Z and 0–9 (plain + Ctrl)
	 *  - F1–F12 (plain + Ctrl + Shift)
	 *  - Escape, Backspace, Delete (plain only)
	 */
	static QList<int> assignableKeys();

	/**
	 * @brief Loads the key→command map from QSettings.
	 * @return A hash keyed on (modifier, key) pairs.
	 */
	static QHash<std::pair<int,int>, TeTypes::CmdId> loadKeyMap();

	/**
	 * @brief Returns true if @p ev represents an assignable key combination.
	 */
	static bool isAssignableKey(const QKeyEvent* ev);
};
