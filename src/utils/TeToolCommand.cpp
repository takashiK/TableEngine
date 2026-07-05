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

#include "TeToolCommand.h"
#include "utils/TeUtils.h"

#include <QProcess>
#include <QFileInfo>
#include <QStringList>

/**
 * @file TeToolCommand.cpp
 * @brief Implementation of TeToolCommand.
 * @ingroup utility
 */

namespace {

	/** @brief Wraps @p value in double quotes when it contains a space. */
	QString quoteIfNeeded(const QString& value)
	{
		return value.contains(QLatin1Char(' ')) ? QString("\"%1\"").arg(value) : value;
	}

} // namespace

bool TeToolCommand::isValidFormat(const QString& commandTemplate)
{
	if (commandTemplate.trimmed().isEmpty()) {
		return true; // Not configured yet - treated as valid.
	}
	const QStringList tokens = QProcess::splitCommand(commandTemplate);
	return !tokens.isEmpty() && !tokens.first().isEmpty();
}

QString TeToolCommand::expandMacros(const QString& commandTemplate, TeViewStore* p_store)
{
	QString result = commandTemplate;

	if (result.contains(QLatin1String("%F")) || result.contains(QLatin1String("%f"))) {
		const QString current = getCurrentItem(p_store);
		if (result.contains(QLatin1String("%F"))) {
			result.replace(QLatin1String("%F"), quoteIfNeeded(current));
		}
		if (result.contains(QLatin1String("%f"))) {
			result.replace(QLatin1String("%f"), quoteIfNeeded(QFileInfo(current).fileName()));
		}
	}

	if (result.contains(QLatin1String("%M")) || result.contains(QLatin1String("%m"))) {
		QStringList selected;
		getSelectedItemList(p_store, &selected);

		if (result.contains(QLatin1String("%M"))) {
			QStringList quoted;
			for (const QString& path : selected) {
				quoted << quoteIfNeeded(path);
			}
			result.replace(QLatin1String("%M"), quoted.join(QLatin1Char(' ')));
		}
		if (result.contains(QLatin1String("%m"))) {
			QStringList names;
			for (const QString& path : selected) {
				names << quoteIfNeeded(QFileInfo(path).fileName());
			}
			result.replace(QLatin1String("%m"), names.join(QLatin1Char(' ')));
		}
	}

	if (result.contains(QLatin1String("%P"))) {
		result.replace(QLatin1String("%P"), quoteIfNeeded(getCurrentFolder(p_store)));
	}

	return result;
}

QString TeToolCommand::workingDirectory(TeViewStore* p_store)
{
	return getCurrentFolder(p_store);
}
