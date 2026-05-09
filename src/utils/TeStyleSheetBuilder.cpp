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

#include "TeStyleSheetBuilder.h"

#include "TeSettings.h"

#include <QSettings>

/**
 * @file TeStyleSheetBuilder.cpp
 * @brief Implementation of TeStyleSheetBuilder helpers.
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
static QString buildBlock(const QString& selector,
                          const QColor& fg, const QColor& bg, bool bold,
                          bool boldIsSet)
{
	QString props;
	appendColor(props, "color",            fg);
	appendColor(props, "background-color", bg);
	if (boldIsSet)
		props += QStringLiteral("    font-weight: %1;\n")
		         .arg(bold ? QStringLiteral("bold") : QStringLiteral("normal"));

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

	const QString family = s.value(QLatin1String(SETTING_FOLDER_FONT_FAMILY)).toString();
	if (!family.isEmpty())
		a.fontFamily = family;

	const int size = s.value(QLatin1String(SETTING_FOLDER_FONT_SIZE), -1).toInt();
	if (size > 0)
		a.fontSize = size;

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
	a.normalBold = hasSetting(SETTING_FOLDER_NORMAL_BOLD) ? readBool(SETTING_FOLDER_NORMAL_BOLD) : false;

	a.selectedFg   = readColor(SETTING_FOLDER_SELECTED_FG);
	a.selectedBg   = readColor(SETTING_FOLDER_SELECTED_BG);
	a.selectedBold = hasSetting(SETTING_FOLDER_SELECTED_BOLD) ? readBool(SETTING_FOLDER_SELECTED_BOLD) : false;

	a.focusFg   = readColor(SETTING_FOLDER_FOCUS_FG);
	a.focusBg   = readColor(SETTING_FOLDER_FOCUS_BG);
	a.focusBold = hasSetting(SETTING_FOLDER_FOCUS_BOLD) ? readBool(SETTING_FOLDER_FOCUS_BOLD) : false;

	a.accentColor = readColor(SETTING_FOLDER_ACCENT_COLOR);

	const QString priority = s.value(QLatin1String(SETTING_FOLDER_FOCUS_PRIORITY),
	                                 QStringLiteral("focus")).toString();
	a.focusPriority = (priority == QStringLiteral("selected"))
	                  ? SelectedFirst : FocusFirst;

	return a;
}

QString TeFolderAppearance::toCss() const
{
	// Both QTreeView and QListView share the same rules.
	const QStringList bases = {
		QStringLiteral("QTreeView::item"),
		QStringLiteral("QListView::item"),
	};

	auto sel = [&bases](const QString& pseudo) -> QString {
		QStringList result;
		for (const auto& base : bases)
			result << base + pseudo;
		return result.join(QStringLiteral(", "));
	};

	QString css;

	// ① Font (applies to the base item selector, shared by all states)
	if (!fontFamily.isEmpty() || fontSize > 0) {
		QString fontProps;
		if (!fontFamily.isEmpty())
			fontProps += QStringLiteral("    font-family: \"%1\";\n").arg(fontFamily);
		if (fontSize > 0)
			fontProps += QStringLiteral("    font-size: %1pt;\n").arg(fontSize);
		if (!fontProps.isEmpty())
			css += sel(QString{}) + QStringLiteral(" {\n") + fontProps + QStringLiteral("}\n");
	}

	// ② Normal state
	const bool hasNormalBold = normalBg.isValid() || normalFg.isValid();
	css += buildBlock(sel(QString{}), normalFg, normalBg, normalBold, hasNormalBold && normalBold);

	// ③ Selected / Focus — order controlled by focusPriority
	auto selectedBlock = buildBlock(sel(QStringLiteral(":selected")),
	                                selectedFg, selectedBg, selectedBold,
	                                selectedFg.isValid() || selectedBg.isValid());
	auto focusBlock    = buildBlock(sel(QStringLiteral(":focus")),
	                                focusFg, focusBg, focusBold,
	                                focusFg.isValid() || focusBg.isValid());

	if (focusPriority == FocusFirst) {
		// ":focus" written last → focus style wins when both active
		css += selectedBlock;
		css += focusBlock;
	} else {
		// ":selected" written last → selected style wins when both active
		css += focusBlock;
		css += selectedBlock;
	}

	// ④ Accent colour (list-mode selection marker)
	// Exposed as a custom property that TeFileItemDelegate reads at paint time.
	// We also set it as a QListView border-left indicator via a sub-element
	// selector as a CSS-only approximation.
	if (accentColor.isValid()) {
		css += QStringLiteral("QListView {\n"
		                      "    qproperty-accentColor: %1;\n"
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
