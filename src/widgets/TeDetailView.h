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
#include <QList>

class QScrollArea;
class QVBoxLayout;
class QToolButton;
class TeDetailSection;

/**
 * @file TeDetailView.h
 * @brief Composite detail panel composed of collapsible TeDetailSection widgets.
 * @ingroup widgets
 */

/**
 * @class TeDetailView
 * @brief Right-hand detail panel that shows metadata sections for the current file.
 * @ingroup widgets
 *
 * @details TeDetailView owns a QScrollArea that holds a vertical stack of
 * (header, section) pairs.  Each header is a checkable QToolButton: when
 * unchecked the corresponding section is hidden (collapsed).
 *
 * Sections are registered with registerSection().  When setFileInfo() is
 * called, every section that returns true from canHandle() is shown and
 * loaded; others are hidden.  The recommended section order is:
 *   1. TeDetailThumbnailSection
 *   2. TeDetailFileInfoSection
 *   3. TeDetailExifSection
 *   4. TeDetailTextPreviewSection
 *
 * New section types can be added at runtime via registerSection().
 *
 * @see TeDetailSection, TeViewStore
 */
class TeDetailView : public QWidget
{
    Q_OBJECT

public:
    explicit TeDetailView(QWidget* parent = nullptr);
    ~TeDetailView() override = default;

    /**
     * @brief Appends @p section to the panel.
     *
     * The view takes ownership of @p section.
     * @param section The section to add.
     */
    void registerSection(TeDetailSection* section);

    QScrollArea* scrollArea() const { return mp_scroll; }
    
public slots:
    /**
     * @brief Updates the panel to show information for @p info.
     * @param info The currently selected file.
     */
    void setFileInfo(const QFileInfo& info);

    /** @brief Clears all sections. */
    void clear();

protected:
    void showEvent(QShowEvent* event) override;

private:
    void applyFileInfo();

    struct Entry {
        QToolButton*    header;
        TeDetailSection* section;
    };

    QScrollArea*  mp_scroll     = nullptr;
    QWidget*      mp_container  = nullptr;
    QVBoxLayout*  mp_layout     = nullptr;
    QList<Entry>  m_entries;
    QFileInfo     m_pendingInfo;
    bool          m_pendingDirty = false;
};
