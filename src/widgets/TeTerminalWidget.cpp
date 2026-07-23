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

#include "TeTerminalWidget.h"

#include <ptyqt.h>

TeTerminalWidget::TeTerminalWidget(QWidget* parent) : QPlainTextEdit(parent) {
    setReadOnly(true);
    mp_ptyProcess = PtyQt::createPtyProcess(IPtyProcess::PtyType::AutoPty);
    mp_decoder = new QStringDecoder(QStringDecoder::Utf8);


    // Connect the aboutToClose signal of the notifier to a lambda function that sets m_isRunning to false
    connect(mp_ptyProcess->notifier(), &QIODevice::aboutToClose, this, [this]() {
        m_isRunning = false;
        emit aboutToClose();
    });

    connect(mp_ptyProcess->notifier(), &QIODevice::readyRead, this, [this]() {
        QByteArray data = mp_ptyProcess->readAll();
        textCursor().insertText(mp_decoder->decode(data));
    });

    loadSettings();
}

TeTerminalWidget::~TeTerminalWidget() {
    delete mp_ptyProcess;
    mp_ptyProcess = nullptr;
    delete mp_decoder;
    mp_decoder = nullptr;
}

bool TeTerminalWidget::startProcess(const QString& executable, const QStringList& arguments, const QString& workingDir, QStringList environment) {
    if (isRunning()) {
        return false; // Process is already running
    }
    return mp_ptyProcess->startProcess(executable, arguments, workingDir, environment, 80, 24);
}

bool TeTerminalWidget::isRunning() const {
    return m_isRunning;
}

void TeTerminalWidget::loadSettings() {
}

void TeTerminalWidget::setANSIByteArray(const QByteArray& byteArray) {
    if(isRunning()) {
        return; // Do not write to the process if it's running  
    }
    setPlainText(mp_decoder->decode(byteArray));
}

void TeTerminalWidget::exitProcess() {
    if(mp_ptyProcess && m_isRunning) {
        mp_ptyProcess->kill();
        m_isRunning = false;
    }
}
