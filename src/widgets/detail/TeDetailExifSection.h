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
#include "utils/TeExifReader.h"

#include <QFileInfo>
#include <QRunnable>
#include <QMap>
#include <QString>
#include <memory>

class QFormLayout;

/**
 * @file TeDetailExifSection.h
 * @brief Detail section that displays EXIF / image metadata.
 * @ingroup widgets
 */

/**
 * @class TeDetailExifSection
 * @brief Shows camera EXIF metadata and image dimensions for image files.
 * @ingroup widgets
 *
 * @details canHandle() returns true for files that QImageReader supports.
 * The actual metadata extraction is performed on a thread-pool thread via
 * TeExifReaderTask, using the injected TeExifReader strategy.
 * The default strategy is TeQImageExifReader; replace it with
 * setExifReader() to plug in exiv2 or any other backend.
 *
 * @see TeDetailSection, TeExifReader, TeQImageExifReader
 */
class TeDetailExifSection : public TeDetailSection
{
    Q_OBJECT

public:
    explicit TeDetailExifSection(QWidget* parent = nullptr);

    /**
     * @brief Replaces the EXIF reading strategy.
     * @param reader New strategy (ownership is transferred).
     */
    void setExifReader(std::unique_ptr<TeExifReader> reader);

    bool    canHandle(const QFileInfo& info) const override;
    void    load(const QFileInfo& info) override;
    void    clear() override;
    QString title() const override;

private slots:
    void onExifReady(const QString& filePath, const QMap<QString,QString>& data);

private:
    void populate(const QMap<QString,QString>& data);

    QFormLayout*                mp_form = nullptr;
    std::unique_ptr<TeExifReader> mp_reader;
    QString                     m_currentPath;
};

// ---------------------------------------------------------------------------
// Worker task
// ---------------------------------------------------------------------------
class TeExifReaderTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    TeExifReaderTask(const QString& path, const TeExifReader* reader);
    void run() override;

signals:
    void exifReady(const QString& filePath, const QMap<QString,QString>& data);

private:
    QString            m_path;
    const TeExifReader* mp_reader;  // not owned
};
