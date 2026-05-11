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

#include "TeFolderAppearance.h"

#include "TeSettings.h"

#include <QSettings>

/**
 * @file TeFolderAppearance.cpp
 * @brief Implementation of TeFolderAppearance helpers.
 * @ingroup utility
 */

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

namespace {

/** Appends "property: value;\n" only when the colour is valid. */
static void appendColor(QString& block, const char* prop, const QColor& color)
{
	if (color.isValid())
		block += QStringLiteral("    %1: %2;\n").arg(QLatin1String(prop), color.name());
}

/**
 * @brief Builds one CSS rule block for the given selectors.
 *
 * Returns an empty string when no property has a valid value (so that empty
 * rule blocks are not written into the stylesheet).
 */
static QString buildBlock(const QString& selector, const QColor& bg)
{
	QString props;
	appendColor(props, "background-color", bg);

	if (props.isEmpty())
		return {};

	return selector + QStringLiteral(" {\n") + props + QStringLiteral("}\n");
}

} // anonymous namespace

// ---------------------------------------------------------------------------
// TeFolderAppearance
// ---------------------------------------------------------------------------

TeFolderAppearance TeFolderAppearance::fromSettings()
{
	QSettings s;
	TeFolderAppearance a;

    a.prio_stylesheet = s.value(QLatin1String(SETTING_FOLDER_PRIO_STYLESHEET), true).toBool();

	const QString family = s.value(QLatin1String(SETTING_FOLDER_FONT_FAMILY)).toString();
	const int size = s.value(QLatin1String(SETTING_FOLDER_FONT_SIZE), -1).toInt();
	if (!family.isEmpty())
		a.font.setFamily(family);
	if (size > 0)
		a.font.setPointSize(size);

	auto readColor = [&s](const char* key) -> QColor {
		const QString v = s.value(QLatin1String(key)).toString();
		return v.isEmpty() ? QColor{} : QColor(v);
	};
	auto readBool = [&s](const char* key, bool defaultVal = false) -> bool {
		return s.value(QLatin1String(key), defaultVal).toBool();
	};
	auto hasSetting = [&s](const char* key) -> bool {
		return s.contains(QLatin1String(key));
	};

	a.normalFg   = readColor(SETTING_FOLDER_NORMAL_FG);
	a.normalBg   = readColor(SETTING_FOLDER_NORMAL_BG);

	a.selectedFg   = readColor(SETTING_FOLDER_SELECTED_FG);
	a.selectedBg   = readColor(SETTING_FOLDER_SELECTED_BG);

	a.focusBg   = readColor(SETTING_FOLDER_FOCUS_BG);

	a.accentColor = readColor(SETTING_FOLDER_ACCENT_COLOR);

	const QString priority = s.value(QLatin1String(SETTING_FOLDER_FOCUS_PRIORITY),
	                                 QStringLiteral("focus")).toString();
	a.focusPriority = (priority == QStringLiteral("selected"))
	                  ? SelectedFirst : FocusFirst;

	return a;
}

QString TeFolderAppearance::toCss() const
{
    // If prio_stylesheet is true, this struct does not effect any styles.
    if (prio_stylesheet) {
        return QString();
    }

	QString css;

	const QString family = font.family();
	const int     size   = font.pointSize();

	// ① Font — QListView sub-element level
	if (!family.isEmpty() || size > 0) {
		QString fontProps;
		if (!family.isEmpty())
			fontProps += QStringLiteral("    font-family: \"%1\";\n").arg(family);
		if (size > 0)
			fontProps += QStringLiteral("    font-size: %1pt;\n").arg(size);
		if (normalFg.isValid())
			fontProps += QStringLiteral("    color: %1;\n").arg(normalFg.name());
		if (selectedFg.isValid())
			fontProps += QStringLiteral("    selection-color: %1;\n").arg(selectedFg.name());	
		css += QStringLiteral("QListView {\n") + fontProps + QStringLiteral("}\n");
	}

	// ② Normal state — QListView::item sub-element
	css += buildBlock(QStringLiteral("QListView::item"),normalBg);

	// ③ Selected / Focus — QListView::item sub-element only; order controlled by focusPriority
	auto selectedBlock = buildBlock(QStringLiteral("QListView::item:selected"), selectedBg);
	auto focusBlock    = buildBlock(QStringLiteral("QListView::item:focus"), focusBg);

	if (focusPriority == FocusFirst) {
		// ":focus" written last → focus style wins when both active
		css += selectedBlock;
		css += focusBlock;
	} else {
		// ":selected" written last → selected style wins when both active
		css += focusBlock;
		css += selectedBlock;
	}

	// ④ QTreeView — widget-level rules only (same values as QListView, no sub-element selectors)
	{
		QString props;
		if (!family.isEmpty())
			props += QStringLiteral("    font-family: \"%1\";\n").arg(family);
		if (size > 0)
			props += QStringLiteral("    font-size: %1pt;\n").arg(size);
		appendColor(props, "background-color", normalBg);
		appendColor(props, "color",            normalFg);
		appendColor(props, "selection-color",  selectedFg);
		if (!props.isEmpty())
			css += QStringLiteral("QTreeView {\n") + props + QStringLiteral("}\n");
	}

	// ⑤ Accent colour (list-mode selection marker)
	// Exposed as a custom property that TeFileItemDelegate reads at paint time.
	if (accentColor.isValid()) {
		css += QStringLiteral("QListView {\n"
		                      "    accent-color: %1;\n"
		                      "}\n").arg(accentColor.name());
	}
	return css;
}

// ---------------------------------------------------------------------------
// buildStyleSheetFromSettings
// ---------------------------------------------------------------------------

QString buildStyleSheetFromSettings()
{
	QString result;

	// --- Folder view appearance ---
	const QString folderCss = TeFolderAppearance::fromSettings().toCss();
	if (!folderCss.isEmpty())
		result += folderCss;

	// --- Add new settings categories here in the future ---

	return result;
}

// ---------------------------------------------------------------------------
// TeFolderAppearance::toSettings
// ---------------------------------------------------------------------------

void TeFolderAppearance::toSettings() const
{
	QSettings s;
	s.setValue(QLatin1String(SETTING_FOLDER_PRIO_STYLESHEET), prio_stylesheet);

	const QString family = font.family();
	const int     size   = font.pointSize();
	if (!family.isEmpty())
		s.setValue(QLatin1String(SETTING_FOLDER_FONT_FAMILY), family);
	else
		s.remove(QLatin1String(SETTING_FOLDER_FONT_FAMILY));
	if (size > 0)
		s.setValue(QLatin1String(SETTING_FOLDER_FONT_SIZE), size);
	else
		s.remove(QLatin1String(SETTING_FOLDER_FONT_SIZE));

	auto writeColor = [&s](const char* key, const QColor& c) {
		if (c.isValid()) s.setValue(QLatin1String(key), c.name());
		else             s.remove(QLatin1String(key));
	};

	writeColor(SETTING_FOLDER_NORMAL_FG,   normalFg);
	writeColor(SETTING_FOLDER_NORMAL_BG,   normalBg);

	writeColor(SETTING_FOLDER_SELECTED_FG, selectedFg);
	writeColor(SETTING_FOLDER_SELECTED_BG, selectedBg);

	writeColor(SETTING_FOLDER_FOCUS_BG,    focusBg);

	writeColor(SETTING_FOLDER_ACCENT_COLOR, accentColor);

	s.setValue(QLatin1String(SETTING_FOLDER_FOCUS_PRIORITY),
	           focusPriority == SelectedFirst
	           ? QStringLiteral("selected") : QStringLiteral("focus"));
}
