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

#include "TeKeyMap.h"
#include "TeSettings.h"

#include <QKeyEvent>
#include <QKeySequence>
#include <QSet>
#include <QSettings>

/**
 * @file TeKeyMap.cpp
 * @brief Implementation of TeKeyMap.
 * @ingroup main
 */

QList<int> TeKeyMap::assignableKeys()
{
	QList<int> list;

	for (uint32_t key = Qt::Key_A; key <= Qt::Key_Z; key++) {
		list << (int)key << ((int)Qt::CTRL + (int)key);
	}
	for (uint32_t key = Qt::Key_0; key <= Qt::Key_9; key++) {
		list << (int)key << ((int)Qt::CTRL + (int)key);
	}
	for (uint32_t key = Qt::Key_F1; key <= Qt::Key_F12; key++) {
		list << (int)key << ((int)Qt::CTRL + (int)key) << ((int)Qt::SHIFT + (int)key);
	}

	list << Qt::Key_Escape << Qt::Key_Backspace << Qt::Key_Delete;
    list << Qt::Key_Tab << (Qt::CTRL + Qt::Key_Tab);

	return list;
}

QHash<std::pair<int,int>, TeTypes::CmdId> TeKeyMap::loadKeyMap()
{
	QHash<std::pair<int,int>, TeTypes::CmdId> map;

	const int modifierMask = Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::META;

	QSettings settings;
	settings.beginGroup(SETTING_KEY);

	for (const int key : assignableKeys()) {
		const int cmdId = settings.value(QKeySequence(key).toString().replace("+", "_")).toInt();
		if (cmdId != TeTypes::CMDID_NONE) {
			map.insert({key & modifierMask, key & ~modifierMask},
			           static_cast<TeTypes::CmdId>(cmdId));
		}
	}

	return map;
}

bool TeKeyMap::isAssignableKey(const QKeyEvent* ev)
{
	static const QSet<int> s_keys = []() {
		QSet<int> s;
		for (const int k : TeKeyMap::assignableKeys())
			s.insert(k);
		return s;
	}();

	const int modifierMask = Qt::CTRL | Qt::SHIFT | Qt::ALT | Qt::META;
	const int combo = (int)(ev->modifiers() & modifierMask) | ev->key();

	return s_keys.contains(combo);
}
