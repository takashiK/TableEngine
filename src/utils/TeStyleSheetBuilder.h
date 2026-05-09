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

#include <QString>
#include <QColor>

/**
 * @file TeStyleSheetBuilder.h
 * @brief Helpers that build a Qt stylesheet string from QSettings values.
 * @ingroup utility
 *
 * @details
 * Each settings section is represented by a dedicated struct with a static
 * fromSettings() factory and a toCss() serialiser.  Fields use "invalid /
 * sentinel" defaults so that only explicitly configured values are emitted
 * into the CSS — unset fields are left to the base stylesheet.
 *
 * To add a new settings category:
 *  1. Add a new struct with fromSettings() / toCss().
 *  2. Call toCss() from buildStyleSheetFromSettings().
 */

/**
 * @brief Appearance settings for QTreeView / QListView folder views.
 *
 * Loaded from the "folder/appearance" QSettings group via fromSettings().
 * toCss() generates CSS rules for QTreeView::item and QListView::item,
 * respecting the focusPriority setting to control which pseudo-state wins
 * when an item is simultaneously selected and focused.
 */
struct TeFolderAppearance
{
	/**
	 * @brief Controls CSS rule order for the ":selected" vs ":focus" pseudo-states.
	 *
	 * Because Qt stylesheet uses CSS cascade order for equal-specificity rules,
	 * the pseudo-state block written *last* in the stylesheet wins when both
	 * states are active at the same time.
	 *
	 * - FocusFirst  (default): ":focus" block is written after ":selected" → focus style wins.
	 * - SelectedFirst        : ":selected" block is written after ":focus"  → selected style wins.
	 */
	enum FocusPriority { FocusFirst, SelectedFirst };

	QString fontFamily;            ///< isEmpty() means "not set"
	int     fontSize   = -1;       ///< -1 means "not set"

	QColor  normalFg,   normalBg;  ///< QColor::isValid()==false means "not set"
	bool    normalBold   = false;

	QColor  selectedFg, selectedBg;
	bool    selectedBold = false;

	QColor  focusFg,    focusBg;
	bool    focusBold    = false;

	QColor  accentColor;           ///< List-mode selection marker colour

	FocusPriority focusPriority = FocusFirst;

	/** @brief Constructs a TeFolderAppearance from the current QSettings values. */
	static TeFolderAppearance fromSettings();

	/**
	 * @brief Serialises the appearance to a Qt stylesheet string.
	 *
	 * Only fields that have been explicitly set (valid colour / non-sentinel
	 * value) are emitted.  The generated rules target both QTreeView::item
	 * and QListView::item.
	 */
	QString toCss() const;
};

/**
 * @brief Builds a complete stylesheet string from all QSettings appearance sections.
 *
 * This is the single entry point called by TeViewStore::applyStyleSheet().
 * It aggregates toCss() from every appearance struct.  Add new categories here.
 */
QString buildStyleSheetFromSettings();
