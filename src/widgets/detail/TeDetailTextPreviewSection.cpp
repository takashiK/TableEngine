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

#include "TeDetailTextPreviewSection.h"
#include "utils/TeUtils.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QFile>
#include <QStringDecoder>
#include <QThreadPool>
#include <QMetaObject>
#include <QFontDatabase>

/**
 * @file TeDetailTextPreviewSection.cpp
 * @brief Implementation of TeDetailTextPreviewSection.
 * @ingroup widgets
 */

// ---------------------------------------------------------------------------
// TeTextPreviewTask
// ---------------------------------------------------------------------------
TeTextPreviewTask::TeTextPreviewTask(const QString& path, QObject* receiver,
                                     const char* slot, int maxLines, int maxBytes)
    : m_path(path)
    , m_maxLines(maxLines)
    , m_maxBytes(maxBytes)
{
    // Wire the signal so results are delivered to the GUI thread
    connect(this, SIGNAL(previewReady(QString,QString)),
            receiver, slot, Qt::QueuedConnection);
}

void TeTextPreviewTask::run()
{
    QFile file(m_path);
    if (!file.open(QFile::ReadOnly)) {
        emit previewReady(m_path, QString());
        return;
    }

    const QByteArray raw = file.read(m_maxBytes);
    file.close();

    // Detect codec (same list as TeDocument)
    static const QStringList kCodecList{
        QStringLiteral("UTF-8"),
        QStringLiteral("EUC-JP"),
        QStringLiteral("Shift_JIS"),
        QStringLiteral("ISO-2022-JP"),
        QStringLiteral("UTF-16LE"),
        QStringLiteral("UTF-16BE"),
    };
    const QString codecName = detectTextCodec(raw, kCodecList);
    const QByteArray codecNameBytes = codecName.isEmpty() ? QByteArray("UTF-8") : codecName.toLatin1();
        QStringDecoder decoder(codecNameBytes.constData());
        if (!decoder.isValid()) {
            decoder = QStringDecoder("UTF-8");
        }
        const QString text = decoder(raw);

    // Keep only the first m_maxLines lines
    const QStringList lines = text.split(QLatin1Char('\n'));
    const QString preview =
        lines.mid(0, qMin(m_maxLines, lines.size())).join(QLatin1Char('\n'));

    emit previewReady(m_path, preview);
}

// ---------------------------------------------------------------------------
// TeDetailTextPreviewSection
// ---------------------------------------------------------------------------
TeDetailTextPreviewSection::TeDetailTextPreviewSection(QWidget* parent)
    : TeDetailSection(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    mp_edit = new QPlainTextEdit(this);
    mp_edit->setReadOnly(true);
    mp_edit->setMaximumHeight(260);

    // Use a monospace font for the preview
    QFont mono = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    mono.setPointSize(9);
    mp_edit->setFont(mono);
    mp_edit->setLineWrapMode(QPlainTextEdit::NoWrap);

    layout->addWidget(mp_edit);
    setLayout(layout);
}

bool TeDetailTextPreviewSection::canHandle(const QFileInfo& info) const
{
    if (!info.isFile()) return false;
    return getFileType(info.absoluteFilePath()) == TE_FILE_TEXT;
}

void TeDetailTextPreviewSection::load(const QFileInfo& info)
{
    m_currentPath = info.absoluteFilePath();
    mp_edit->setPlaceholderText(tr("Loading…"));
    mp_edit->clear();

    auto* task = new TeTextPreviewTask(
        m_currentPath, this,
        SLOT(onPreviewReady(QString,QString)),
        kMaxLines, kMaxBytes);
    task->setAutoDelete(true);
    QThreadPool::globalInstance()->start(task);
}

void TeDetailTextPreviewSection::clear()
{
    m_currentPath.clear();
    mp_edit->clear();
    mp_edit->setPlaceholderText(QString());
}

QString TeDetailTextPreviewSection::title() const
{
    return tr("Text Preview");
}

void TeDetailTextPreviewSection::onPreviewReady(const QString& filePath,
                                                const QString& text)
{
    if (filePath != m_currentPath) return;
    mp_edit->setPlaceholderText(QString());
    mp_edit->setPlainText(text);
    emit ready();
}
