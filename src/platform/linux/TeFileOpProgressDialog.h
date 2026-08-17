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

#include <QDialog>

/**
 * @file TeFileOpProgressDialog.h
 * @brief Linux-only, GUI-thread progress dialog for file operations.
 * @ingroup platform
 */

class QLabel;
class QProgressBar;
class QPushButton;
class QTimer;
class QCloseEvent;
class TeFileOpProgress;

namespace TeFileOpProgressDialogPrivate { class RateGraph; }

/**
 * @class TeFileOpProgressDialog
 * @brief Non-modal progress dialog shown while a Linux file operation runs
 *        on TeFileOperationManager's worker thread.
 * @ingroup platform
 *
 * @details Must be constructed, shown, and destroyed on the GUI thread. It
 * polls a TeFileOpProgress snapshot with a QTimer instead of relying on
 * cross-thread signals, so neither the worker thread nor this dialog ever
 * blocks waiting on the other. Clicking Cancel calls TeFileOpProgress::requestCancel(),
 * which the worker-thread file operation observes and aborts on.
 */
class TeFileOpProgressDialog : public QDialog
{
	Q_OBJECT
public:
	TeFileOpProgressDialog(TeFileOpProgress* progress, const QString& title, QWidget* parent = nullptr);
	~TeFileOpProgressDialog() override;

	/** @brief Detaches the TeFileOpProgress this dialog polls, and suppresses
	 *         the delayed initial show(). Must be called by the owner before
	 *         destroying the paired TeFileOpProgress, so neither a queued
	 *         poll() nor the delayed show() (which can still be pending during
	 *         a nested QMessageBox event loop) can touch it afterwards. */
	void detachProgress();

protected:
	void closeEvent(QCloseEvent* event) override;
	/** @brief Escape triggers reject() rather than closeEvent(); route it
	 *         through the same cancellation path as Cancel/close. */
	void reject() override;

private slots:
	void poll();

private:
	/** @brief Requests cancellation and disables the Cancel button; shared by
	 *         the button click handler and closeEvent() so closing the window
	 *         behaves exactly like pressing Cancel. */
	void requestCancellation();

	TeFileOpProgress*                    mp_progress;
	bool                                 m_detached = false;
	QLabel*                              mp_pathLabel = nullptr;
	QProgressBar*                        mp_progressBar = nullptr;
	QLabel*                              mp_statsLabel = nullptr;
	QPushButton*                         mp_cancelButton = nullptr;
	TeFileOpProgressDialogPrivate::RateGraph* mp_rateGraph = nullptr;
	QTimer*                              mp_timer = nullptr;
	qint64                               m_lastProcessedBytes = 0;
	qint64                               m_lastElapsedMs = 0;
};
