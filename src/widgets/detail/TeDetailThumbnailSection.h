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
#include "utils/TeImageLoader.h"

#include <QFileInfo>

class QLabel;

/**
 * @file TeDetailThumbnailSection.h
 * @brief Detail section that renders a 192 × 192 px thumbnail for image files.
 * @ingroup widgets
 */

/**
 * @class TeDetailThumbnailSection
 * @brief Shows a scaled thumbnail at the top of the detail panel for supported
 *        image file formats.
 * @ingroup widgets
 *
 * @details canHandle() returns true when QImageReader recognises the file
 * extension.  load() first displays an OS-cached icon for instant feedback
 * (via TeThumbnailProvider), then requests a full-quality 192 × 192 decode
 * through TeImageLoader.  When the async task completes, the label is updated
 * and ready() is emitted.
 *
 * @see TeDetailSection, TeImageLoader, TeThumbnailProvider
 */
class TeDetailThumbnailSection : public TeDetailSection
{
    Q_OBJECT

public:
    explicit TeDetailThumbnailSection(QWidget* parent = nullptr);

    bool    canHandle(const QFileInfo& info) const override;
    void    load(const QFileInfo& info) override;
    void    clear() override;
    QString title() const override;

private slots:
    void onImageReady(const QString& filePath);

private:
    static const QSize kThumbSize;

    QLabel*        mp_label       = nullptr;
    TeImageLoader* mp_loader      = nullptr;
    QString        m_currentPath;
};
