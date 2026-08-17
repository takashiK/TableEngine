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

#include "platform/TeFileOperationManager.h"
#include "platform/platform_util.h"
#include "platform/TeFileOpProgress.h"

#include <QThread>
#include <QWidget>
#include <QMessageBox>
#include <QIcon>

#include <utility>

#ifdef Q_OS_LINUX
#include "platform/linux/TeFileOpProgressDialog.h"
#endif

/**
 * @file TeFileOperationManager.cpp
 * @brief Implementation of TeFileOperationManager.
 * @ingroup platform
 */

namespace {
	/**
	 * @brief Worker thread that initializes a per-thread COM apartment so shell
	 *        file operations can run off the GUI thread.
	 */
	class TeFileOpThread : public QThread
	{
	public:
		using QThread::QThread;

	protected:
		void run() override
		{
			comInitializeThread();
			exec();
			comUninitializeThread();
		}
	};
}

//////////////////////////////////////////////////////////////
//
// TeFileOpWorker
//

TeFileOpWorker::TeFileOpWorker(QObject* parent)
	: QObject(parent)
{
}

void TeFileOpWorker::doCopyFiles(quint64 id, const QStringList& files, const QString& path, quint64 owner, quint64 progress)
{
	bool success = ::copyFiles(files, path, static_cast<WId>(owner), reinterpret_cast<TeFileOpProgress*>(progress));
	emit operationFinished(id, success);
}

void TeFileOpWorker::doCopyFile(quint64 id, const QString& fromFile, const QString& toFile, quint64 owner, quint64 progress)
{
	bool success = ::copyFile(fromFile, toFile, static_cast<WId>(owner), reinterpret_cast<TeFileOpProgress*>(progress));
	emit operationFinished(id, success);
}

void TeFileOpWorker::doMoveFiles(quint64 id, const QStringList& files, const QString& path, quint64 owner, quint64 progress)
{
	bool success = ::moveFiles(files, path, static_cast<WId>(owner), reinterpret_cast<TeFileOpProgress*>(progress));
	emit operationFinished(id, success);
}

void TeFileOpWorker::doDeleteFiles(quint64 id, const QStringList& files, quint64 owner, quint64 progress)
{
	bool success = ::deleteFiles(files, static_cast<WId>(owner), reinterpret_cast<TeFileOpProgress*>(progress));
	emit operationFinished(id, success);
}

//////////////////////////////////////////////////////////////
//
// TeFileOperationManager
//

TeFileOperationManager::TeFileOperationManager(QObject* parent)
	: QObject(parent)
{
	mp_worker = new TeFileOpWorker;
	mp_thread = new TeFileOpThread(this);
	mp_worker->moveToThread(mp_thread);

	connect(mp_worker, &TeFileOpWorker::operationFinished,
	        this, &TeFileOperationManager::onWorkerFinished);

	mp_thread->start();
}

TeFileOperationManager::~TeFileOperationManager()
{
	// Request cancellation of any in-flight operation first, so the worker
	// thread's file operation observes it and returns quickly; otherwise
	// mp_thread->wait() below would block app teardown until a long copy/
	// move/delete finishes on its own.
	for (auto it = m_progress.constBegin(); it != m_progress.constEnd(); ++it) {
		it.value()->requestCancel();
	}

	mp_thread->quit();
	mp_thread->wait();
	// The thread has stopped, so it is safe to delete the worker directly.
	delete mp_worker;

	// Clean up any progress dialogs/reporters left over from operations that
	// never reached onWorkerFinished() (e.g. the manager is destroyed early).
	// A dialog may already have been destroyed by its parent owner widget;
	// QPointer guards against double-deleting/dereferencing a dangling pointer.
#ifdef Q_OS_LINUX
	for (const auto& dialog : std::as_const(m_dialogs)) {
		delete dialog;
	}
#endif
	qDeleteAll(m_progress);
}

void TeFileOperationManager::setOwnerWidget(QWidget* widget)
{
	mp_owner = widget;
}

quint64 TeFileOperationManager::ownerHandle() const
{
	return mp_owner ? static_cast<quint64>(mp_owner->winId()) : 0;
}

TeFileOpProgress* TeFileOperationManager::beginProgress(quint64 id, const QString& title)
{
#ifdef Q_OS_LINUX
	// Linux has no native shell copy/move/delete UI, so a GUI-thread dialog
	// is created here and polls the worker-thread-owned TeFileOpProgress. The
	// dialog shows itself after a short delay so short operations don't flash
	// it on screen.
	TeFileOpProgress* progress = new TeFileOpProgress();
	m_progress.insert(id, progress);
	TeFileOpProgressDialog* dialog = new TeFileOpProgressDialog(progress, title, mp_owner);
	m_dialogs.insert(id, dialog);
	return progress;
#else
	// Windows already shows native progress via IFileOperation.
	Q_UNUSED(id);
	Q_UNUSED(title);
	return nullptr;
#endif
}

void TeFileOperationManager::endProgress(quint64 id)
{
#ifdef Q_OS_LINUX
	const QPointer<TeFileOpProgressDialog> dialog = m_dialogs.take(id);
	// The dialog may already have been destroyed by its parent owner widget;
	// QPointer keeps that case a safe no-op instead of a dangling-pointer call.
	if (dialog) {
		// detachProgress() must run before the TeFileOpProgress below is deleted:
		// dialog->deleteLater() only schedules deletion, and a nested QMessageBox
		// event loop started right after this call returns (see onWorkerFinished())
		// can run long enough for the dialog's delayed initial show() to still
		// fire, or (if the timer were still running) for poll() to read the
		// about-to-be-freed TeFileOpProgress.
		dialog->detachProgress();
		dialog->close();
		dialog->deleteLater();
	}
	delete m_progress.take(id);
#else
	Q_UNUSED(id);
#endif
}

quint64 TeFileOperationManager::copyFiles(const QStringList& files, const QString& path, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	TeFileOpProgress* progress = beginProgress(id, tr("Copying"));
	QMetaObject::invokeMethod(mp_worker, "doCopyFiles", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QStringList, files),
	                          Q_ARG(QString, path), Q_ARG(quint64, ownerHandle()),
	                          Q_ARG(quint64, reinterpret_cast<quint64>(progress)));
	return id;
}

quint64 TeFileOperationManager::copyFile(const QString& fromFile, const QString& toFile, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	TeFileOpProgress* progress = beginProgress(id, tr("Copying"));
	QMetaObject::invokeMethod(mp_worker, "doCopyFile", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QString, fromFile),
	                          Q_ARG(QString, toFile), Q_ARG(quint64, ownerHandle()),
	                          Q_ARG(quint64, reinterpret_cast<quint64>(progress)));
	return id;
}

quint64 TeFileOperationManager::moveFiles(const QStringList& files, const QString& path, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	TeFileOpProgress* progress = beginProgress(id, tr("Moving"));
	QMetaObject::invokeMethod(mp_worker, "doMoveFiles", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QStringList, files),
	                          Q_ARG(QString, path), Q_ARG(quint64, ownerHandle()),
	                          Q_ARG(quint64, reinterpret_cast<quint64>(progress)));
	return id;
}

quint64 TeFileOperationManager::deleteFiles(const QStringList& files, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	TeFileOpProgress* progress = beginProgress(id, tr("Deleting"));
	QMetaObject::invokeMethod(mp_worker, "doDeleteFiles", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QStringList, files),
	                          Q_ARG(quint64, ownerHandle()),
	                          Q_ARG(quint64, reinterpret_cast<quint64>(progress)));
	return id;
}

void TeFileOperationManager::onWorkerFinished(quint64 id, bool success)
{
	// Snapshot cancellation before endProgress() destroys the TeFileOpProgress
	// for this id, so a user-cancelled operation can still be recognized as
	// such afterwards and never pops up a failure message box.
	TeFileOpProgress* progress = m_progress.value(id);
	const bool wasCancelled = (progress != nullptr) && progress->isCancelled();

	endProgress(id);

	if (!success && !wasCancelled) {
		QString text = m_errorText.value(id);
		if (!text.isEmpty()) {
			QMessageBox msg(mp_owner);
			msg.setIconPixmap(QIcon(":TableEngine/warning.png").pixmap(32, 32));
			msg.setText(text);
			msg.exec();
		}
	}
	m_errorText.remove(id);

	emit operationFinished(id, success);
}
