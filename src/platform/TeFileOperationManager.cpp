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

#include <QThread>
#include <QWidget>
#include <QMessageBox>
#include <QIcon>

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

void TeFileOpWorker::doCopyFiles(quint64 id, const QStringList& files, const QString& path, quint64 owner)
{
	bool success = ::copyFiles(files, path, static_cast<WId>(owner));
	emit operationFinished(id, success);
}

void TeFileOpWorker::doCopyFile(quint64 id, const QString& fromFile, const QString& toFile, quint64 owner)
{
	bool success = ::copyFile(fromFile, toFile, static_cast<WId>(owner));
	emit operationFinished(id, success);
}

void TeFileOpWorker::doMoveFiles(quint64 id, const QStringList& files, const QString& path, quint64 owner)
{
	bool success = ::moveFiles(files, path, static_cast<WId>(owner));
	emit operationFinished(id, success);
}

void TeFileOpWorker::doDeleteFiles(quint64 id, const QStringList& files, quint64 owner)
{
	bool success = ::deleteFiles(files, static_cast<WId>(owner));
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
	mp_thread->quit();
	mp_thread->wait();
	// The thread has stopped, so it is safe to delete the worker directly.
	delete mp_worker;
}

void TeFileOperationManager::setOwnerWidget(QWidget* widget)
{
	mp_owner = widget;
}

quint64 TeFileOperationManager::ownerHandle() const
{
	return mp_owner ? static_cast<quint64>(mp_owner->winId()) : 0;
}

quint64 TeFileOperationManager::copyFiles(const QStringList& files, const QString& path, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	QMetaObject::invokeMethod(mp_worker, "doCopyFiles", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QStringList, files),
	                          Q_ARG(QString, path), Q_ARG(quint64, ownerHandle()));
	return id;
}

quint64 TeFileOperationManager::copyFile(const QString& fromFile, const QString& toFile, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	QMetaObject::invokeMethod(mp_worker, "doCopyFile", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QString, fromFile),
	                          Q_ARG(QString, toFile), Q_ARG(quint64, ownerHandle()));
	return id;
}

quint64 TeFileOperationManager::moveFiles(const QStringList& files, const QString& path, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	QMetaObject::invokeMethod(mp_worker, "doMoveFiles", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QStringList, files),
	                          Q_ARG(QString, path), Q_ARG(quint64, ownerHandle()));
	return id;
}

quint64 TeFileOperationManager::deleteFiles(const QStringList& files, const QString& errorText)
{
	quint64 id = m_nextId++;
	if (!errorText.isEmpty())
		m_errorText.insert(id, errorText);
	QMetaObject::invokeMethod(mp_worker, "doDeleteFiles", Qt::QueuedConnection,
	                          Q_ARG(quint64, id), Q_ARG(QStringList, files),
	                          Q_ARG(quint64, ownerHandle()));
	return id;
}

void TeFileOperationManager::onWorkerFinished(quint64 id, bool success)
{
	if (!success) {
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
