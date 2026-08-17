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

#include "platform/TeFileOpProgress.h"

/**
 * @file TeFileOpProgress.cpp
 * @brief Implementation of TeFileOpProgress.
 * @ingroup platform
 */

void TeFileOpProgress::start(qint64 totalBytes)
{
	QMutexLocker locker(&m_mutex);
	m_processedBytes = 0;
	m_totalBytes = totalBytes;
	m_currentPath.clear();
	m_timer.start();
}

void TeFileOpProgress::addProcessed(qint64 bytes, const QString& currentPath)
{
	QMutexLocker locker(&m_mutex);
	m_processedBytes += bytes;
	m_currentPath = currentPath;
}

void TeFileOpProgress::requestCancel()
{
	m_cancelled.storeRelease(1);
}

bool TeFileOpProgress::isCancelled() const
{
	return m_cancelled.loadAcquire() != 0;
}

TeFileOpProgress::Snapshot TeFileOpProgress::snapshot() const
{
	QMutexLocker locker(&m_mutex);
	Snapshot snap;
	snap.processedBytes = m_processedBytes;
	snap.totalBytes = m_totalBytes;
	snap.currentPath = m_currentPath;
	snap.elapsedMs = m_timer.isValid() ? m_timer.elapsed() : 0;
	return snap;
}
