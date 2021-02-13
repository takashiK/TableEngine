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

#include "ProgressTracker.h"

ProgressTracker::ProgressTracker(QObject *parent)
	: QObject(parent)
{
	clear();
}

ProgressTracker::~ProgressTracker()
{
}

void ProgressTracker::clear()
{
	count = 0;
	maxvalue = 0;
	progress = 0;
	strList.clear();
}

void ProgressTracker::setMaxValue(int value)
{
	maxvalue = value;
}

void ProgressTracker::setProgress(int value)
{
	count++;
	progress = value;
}

void ProgressTracker::setText(const QString& value)
{
	strList.append(value);
}
