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

#include "TeDetailSection.h"

#include <QFileInfo>
#include <QRunnable>
#include <QString>
#include <QStringList>

class QPlainTextEdit;

/**
 * @file TeDetailTextPreviewSection.h
 * @brief Detail section that shows the first lines of a text file.
 * @ingroup widgets
 */

/**
 * @class TeDetailTextPreviewSection
 * @brief Renders the first 20 lines of a plain-text file.
 * @ingroup widgets
 *
 * @details canHandle() uses getFileType() (which combines extension heuristics
 * with libmagic MIME detection) to decide whether the file is textual.  The
 * read and codec-detection are performed on a QThreadPool thread so the UI
 * never blocks.  Uses detectTextCodec() from TeUtils.h for encoding detection.
 *
 * @see TeDetailSection, getFileType, detectTextCodec
 */
class TeDetailTextPreviewSection : public TeDetailSection
{
    Q_OBJECT

public:
    explicit TeDetailTextPreviewSection(QWidget* parent = nullptr);

    bool    canHandle(const QFileInfo& info) const override;
    void    load(const QFileInfo& info) override;
    void    clear() override;
    QString title() const override;

private slots:
    void onPreviewReady(const QString& filePath, const QString& text);

private:
    static const int kMaxLines = 20;
    static const int kMaxBytes = 16384;  // read at most 16 KB

    QPlainTextEdit* mp_edit       = nullptr;
    QString         m_currentPath;
};

// ---------------------------------------------------------------------------
// Worker task (defined in .h to avoid a separate translation unit)
// ---------------------------------------------------------------------------
class TeTextPreviewTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    TeTextPreviewTask(const QString& path, QObject* receiver,
                      const char* slot, int maxLines, int maxBytes);
    void run() override;

signals:
    void previewReady(const QString& filePath, const QString& text);

private:
    QString m_path;
    int     m_maxLines;
    int     m_maxBytes;
};
