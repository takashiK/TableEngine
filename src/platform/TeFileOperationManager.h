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

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>

/**
 * @file TeFileOperationManager.h
 * @brief Background (modeless) shell file-operation manager.
 * @ingroup platform
 *
 * @details Runs copy/move/delete operations on a dedicated worker thread so the
 * main window stays responsive while the shell performs the work.  Operations
 * are processed serially by the worker thread's event queue; submitting a new
 * operation never blocks the GUI thread nor the command dispatcher queue.
 *
 * @see doc/markdown/11_platform.md
 */

class QWidget;
class QThread;

/**
 * @class TeFileOpWorker
 * @brief Worker living on the file-operation thread; executes shell operations.
 * @ingroup platform
 *
 * @details Each slot is invoked via a queued connection from
 * TeFileOperationManager, so calls execute one at a time on the worker thread.
 * The owner window handle is passed as an integer to keep the queued-connection
 * argument types registrable.
 */
class TeFileOpWorker : public QObject
{
	Q_OBJECT
public:
	explicit TeFileOpWorker(QObject* parent = nullptr);

public slots:
	void doCopyFiles(quint64 id, const QStringList& files, const QString& path, quint64 owner);
	void doCopyFile(quint64 id, const QString& fromFile, const QString& toFile, quint64 owner);
	void doMoveFiles(quint64 id, const QStringList& files, const QString& path, quint64 owner);
	void doDeleteFiles(quint64 id, const QStringList& files, quint64 owner);

signals:
	/** @brief Emitted on the worker thread when an operation completes. */
	void operationFinished(quint64 id, bool success);
};

/**
 * @class TeFileOperationManager
 * @brief Submits shell file operations to a background worker thread.
 * @ingroup platform
 *
 * @details Owned by TeViewStore.  Commands obtain it via
 * TeViewStore::fileOperationManager() and submit operations that return
 * immediately.  Failure is reported on the GUI thread via an optional message
 * box (when an error text was supplied) and the operationFinished() signal.
 */
class TeFileOperationManager : public QObject
{
	Q_OBJECT
public:
	explicit TeFileOperationManager(QObject* parent = nullptr);
	~TeFileOperationManager() override;

	/**
	 * @brief Sets the widget used as parent for error dialogs and as the
	 *        shell operation owner window.
	 */
	void setOwnerWidget(QWidget* widget);

	/** @brief Queues a copy of @p files into @p path. Returns the operation id. */
	quint64 copyFiles(const QStringList& files, const QString& path, const QString& errorText = QString());
	/** @brief Queues a copy of @p fromFile to @p toFile. Returns the operation id. */
	quint64 copyFile(const QString& fromFile, const QString& toFile, const QString& errorText = QString());
	/** @brief Queues a move of @p files into @p path. Returns the operation id. */
	quint64 moveFiles(const QStringList& files, const QString& path, const QString& errorText = QString());
	/** @brief Queues a delete of @p files. Returns the operation id. */
	quint64 deleteFiles(const QStringList& files, const QString& errorText = QString());

signals:
	/** @brief Emitted on the GUI thread after an operation completes. */
	void operationFinished(quint64 id, bool success);

private slots:
	void onWorkerFinished(quint64 id, bool success);

private:
	quint64 ownerHandle() const;

	QThread*        mp_thread = nullptr;
	TeFileOpWorker* mp_worker = nullptr;
	QWidget*        mp_owner  = nullptr;
	quint64         m_nextId  = 1;
	QHash<quint64, QString> m_errorText;
};
