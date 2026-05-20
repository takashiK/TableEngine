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

#include "TeDetailThumbnailSection.h"
#include "utils/TeThumbnailProvider.h"
#include "utils/TeImageLoader.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QImageReader>
#include <QPixmapCache>
#include <QFileInfo>

/**
 * @file TeDetailThumbnailSection.cpp
 * @brief Implementation of TeDetailThumbnailSection.
 * @ingroup widgets
 */

const QSize TeDetailThumbnailSection::kThumbSize{192, 192};

TeDetailThumbnailSection::TeDetailThumbnailSection(QWidget* parent)
    : TeDetailSection(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setAlignment(Qt::AlignHCenter);

    mp_label = new QLabel(this);
    mp_label->setAlignment(Qt::AlignCenter);
    mp_label->setFixedSize(kThumbSize);
    mp_label->setFrameShape(QFrame::StyledPanel);
    layout->addWidget(mp_label);

    mp_loader = new TeImageLoader(this);
    connect(mp_loader, &TeImageLoader::imageReady,
            this, &TeDetailThumbnailSection::onImageReady);

    setLayout(layout);
}

bool TeDetailThumbnailSection::canHandle(const QFileInfo& info) const
{
    if (!info.isFile()) return false;
    const QByteArray fmt = QImageReader::imageFormat(info.absoluteFilePath());
    return !fmt.isEmpty();
}

void TeDetailThumbnailSection::load(const QFileInfo& info)
{
    m_currentPath = info.absoluteFilePath();

    // ── Step 1: try QPixmapCache (previously loaded at this size) ──────────
    const QString key = TeImageLoader::cacheKey(
        m_currentPath, kThumbSize, info.lastModified());
    QPixmap cached;
    if (QPixmapCache::find(key, &cached)) {
        mp_label->setPixmap(cached);
        emit ready();
        return;
    }

    // ── Step 2: show OS icon immediately as placeholder ─────────────────────
    const QIcon icon = TeThumbnailProvider::iconProvider()->icon(info);
    if (!icon.isNull())
        mp_label->setPixmap(icon.pixmap(kThumbSize));
    else
        mp_label->clear();

    // ── Step 3: request a high-quality async decode ─────────────────────────
    mp_loader->requestLoad(m_currentPath, kThumbSize);
}

void TeDetailThumbnailSection::clear()
{
    m_currentPath.clear();
    mp_loader->clearPendingRequests();
    mp_label->clear();
}

QString TeDetailThumbnailSection::title() const
{
    return tr("Preview");
}

void TeDetailThumbnailSection::onImageReady(const QString& filePath)
{
    if (filePath != m_currentPath) return;

    QFileInfo info(filePath);
    const QString key = TeImageLoader::cacheKey(filePath, kThumbSize, info.lastModified());
    QPixmap px;
    if (QPixmapCache::find(key, &px)) {
        mp_label->setPixmap(px);
        emit ready();
    }
}
