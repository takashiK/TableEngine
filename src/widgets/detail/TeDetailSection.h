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

#include <QWidget>
#include <QFileInfo>

/**
 * @file TeDetailSection.h
 * @brief Abstract base class for a collapsible information section in TeDetailView.
 * @ingroup widgets
 */

/**
 * @class TeDetailSection
 * @brief Abstract base for a single named information block inside TeDetailView.
 * @ingroup widgets
 *
 * @details Each concrete subclass handles a specific category of file metadata
 * (basic info, thumbnail, EXIF, text preview, …).  TeDetailView calls
 * canHandle() to decide whether to show the section for the current file, then
 * calls load() to populate it.  Async subclasses emit ready() when the content
 * is available.
 *
 * @see TeDetailView, TeDetailFileInfoSection, TeDetailThumbnailSection,
 *      TeDetailExifSection, TeDetailTextPreviewSection
 */
class TeDetailSection : public QWidget
{
    Q_OBJECT

public:
    explicit TeDetailSection(QWidget* parent = nullptr);
    virtual ~TeDetailSection() = default;

    /**
     * @brief Returns true when this section has content to show for @p info.
     * @param info File to inspect.
     */
    virtual bool    canHandle(const QFileInfo& info) const = 0;

    /**
     * @brief Populates the section for @p info.
     *
     * May be asynchronous; implementations emit ready() when done.
     * @param info File whose metadata should be displayed.
     */
    virtual void    load(const QFileInfo& info) = 0;

    /** @brief Clears all displayed content. */
    virtual void    clear() = 0;

    /** @brief Returns the localised section title shown in the collapsible header. */
    virtual QString title() const = 0;

signals:
    /** @brief Emitted when async loading has finished and the content is ready. */
    void ready();
};
