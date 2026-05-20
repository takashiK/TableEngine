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

#include "TeDetailView.h"
#include "detail/TeDetailSection.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <QToolButton>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSpacerItem>
#include <QShowEvent>

/**
 * @file TeDetailView.cpp
 * @brief Implementation of TeDetailView.
 * @ingroup widgets
 */

TeDetailView::TeDetailView(QWidget* parent)
    : QWidget(parent)
{
    auto* outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    mp_scroll = new QScrollArea(this);
    mp_scroll->setWidgetResizable(true);
    mp_scroll->setFrameShape(QFrame::NoFrame);
    mp_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    mp_container = new QWidget(mp_scroll);
    mp_layout    = new QVBoxLayout(mp_container);
    mp_layout->setContentsMargins(0, 0, 0, 0);
    mp_layout->setSpacing(0);
    mp_layout->addStretch(1);   // pushes sections to the top

    mp_container->setLayout(mp_layout);
    mp_scroll->setWidget(mp_container);
    outerLayout->addWidget(mp_scroll);
    setLayout(outerLayout);

    setMinimumWidth(300);
}

void TeDetailView::registerSection(TeDetailSection* section)
{
    Q_ASSERT(section);

    // ── Create collapsible header button ───────────────────────────────────
    auto* header = new QToolButton(mp_container);
    header->setText(section->title());
    header->setCheckable(true);
    header->setChecked(true);           // expanded by default
    header->setArrowType(Qt::DownArrow);
    header->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    header->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    header->setStyleSheet(QStringLiteral(
        "QToolButton { border: none; text-align: left; padding: 3px 4px; "
        "font-weight: bold; background: palette(mid); }"
        "QToolButton:hover { background: palette(midlight); }"));

    connect(header, &QToolButton::toggled, [header, section](bool checked) {
        section->setVisible(checked);
        header->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);
    });

    // Insert before the trailing stretch (last item)
    const int insertPos = mp_layout->count() - 1;
    mp_layout->insertWidget(insertPos,     header);
    mp_layout->insertWidget(insertPos + 1, section);

    // Start hidden; setFileInfo() will show relevant sections
    header->hide();
    section->hide();

    m_entries.append(Entry{header, section});
}

void TeDetailView::setFileInfo(const QFileInfo& info)
{
    m_pendingInfo  = info;
    m_pendingDirty = true;
    if (isVisible())
        applyFileInfo();
}

void TeDetailView::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    if (m_pendingDirty)
        applyFileInfo();
}

void TeDetailView::applyFileInfo()
{
    m_pendingDirty = false;
    for (const Entry& e : m_entries) {
        if (e.section->canHandle(m_pendingInfo)) {
            e.header->show();
            e.section->load(m_pendingInfo);
            // Restore visibility according to the collapsed state
            e.section->setVisible(e.header->isChecked());
        } else {
            e.header->hide();
            e.section->hide();
            e.section->clear();
        }
    }
}

void TeDetailView::clear()
{
    m_pendingInfo  = QFileInfo();
    m_pendingDirty = false;
    for (const Entry& e : m_entries) {
        e.header->hide();
        e.section->hide();
        e.section->clear();
    }
}
