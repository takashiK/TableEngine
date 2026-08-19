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
#include <QElapsedTimer>
#include <QMutex>
#include <QAtomicInt>

/**
 * @file TeFileOpProgress.h
 * @brief Thread-safe progress state shared between a file-operation worker
 *        thread and a GUI-thread progress dialog.
 * @ingroup platform
 */

/**
 * @class TeFileOpProgress
 * @brief Plain (non-QObject) thread-safe progress counter.
 * @ingroup platform
 *
 * @details The worker thread calls start()/addProcessed() while it performs a
 * file operation. A GUI-thread dialog polls snapshot() (e.g. from a QTimer)
 * to update its widgets and calls requestCancel() when the user cancels.
 * No signals/slots or blocking cross-thread calls are used, so there is no
 * risk of a worker/GUI deadlock and no requirement for either side to run an
 * event loop while the other is active.
 */
class TeFileOpProgress
{
public:
	/** @brief Snapshot of the progress state read by the GUI thread. */
	struct Snapshot {
		qint64  processedBytes = 0;
		qint64  totalBytes = 0;
		QString currentPath;
		qint64  elapsedMs = 0;
	};

	/** @brief Resets counters and starts the elapsed timer. Called once by the worker. */
	void start(qint64 totalBytes);

	/** @brief Adds @p bytes to the processed total and records the item currently being processed. */
	void addProcessed(qint64 bytes, const QString& currentPath);

	/** @brief Requests cancellation; the worker observes this via isCancelled(). */
	void requestCancel();

	/** @brief Returns true once requestCancel() has been called. Safe to poll from the worker thread. */
	bool isCancelled() const;

	/** @brief Returns a consistent copy of the current progress state. */
	Snapshot snapshot() const;

private:
	mutable QMutex m_mutex;
	QElapsedTimer  m_timer;
	qint64         m_processedBytes = 0;
	qint64         m_totalBytes = 0;
	QString        m_currentPath;
	QAtomicInt     m_cancelled { 0 };
};
