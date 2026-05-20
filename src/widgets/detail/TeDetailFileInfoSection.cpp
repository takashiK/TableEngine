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

#include "TeDetailFileInfoSection.h"

#include <QFormLayout>
#include <QLabel>
#include <QFileIconProvider>
#include <QLocale>

/**
 * @file TeDetailFileInfoSection.cpp
 * @brief Implementation of TeDetailFileInfoSection.
 * @ingroup widgets
 */

TeDetailFileInfoSection::TeDetailFileInfoSection(QWidget* parent)
    : TeDetailSection(parent)
{
    auto* layout = new QFormLayout(this);
    layout->setContentsMargins(6, 4, 6, 4);
    layout->setSpacing(3);
    layout->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);

    mp_name     = new QLabel(this);
    mp_type     = new QLabel(this);
    mp_size     = new QLabel(this);
    mp_modified = new QLabel(this);
    mp_path     = new QLabel(this);

    mp_name->setWordWrap(true);
    mp_path->setWordWrap(true);
    mp_path->setTextInteractionFlags(Qt::TextSelectableByMouse);

    // Helper to create bold row-header labels
    auto makeBoldLabel = [this](const QString& text) {
        auto* lbl = new QLabel(text, this);
        QFont f = lbl->font();
        f.setBold(true);
        lbl->setFont(f);
        return lbl;
    };

    layout->addRow(makeBoldLabel(tr("Name:")),     mp_name);
    layout->addRow(makeBoldLabel(tr("Type:")),     mp_type);
    layout->addRow(makeBoldLabel(tr("Size:")),     mp_size);
    layout->addRow(makeBoldLabel(tr("Modified:")), mp_modified);
    layout->addRow(makeBoldLabel(tr("Path:")),     mp_path);

    setLayout(layout);
}

bool TeDetailFileInfoSection::canHandle(const QFileInfo& /*info*/) const
{
    return true;
}

void TeDetailFileInfoSection::load(const QFileInfo& info)
{
    mp_name->setText(info.fileName());

    // Type: use OS description string
    QFileIconProvider prov;
    mp_type->setText(prov.type(info));

    // Size
    if (info.isDir()) {
        mp_size->setText(tr("—"));
    } else {
        const qint64 bytes = info.size();
        const QLocale loc;
        if (bytes < 1024)
            mp_size->setText(tr("%1 B").arg(loc.toString(bytes)));
        else if (bytes < 1024 * 1024)
            mp_size->setText(tr("%1 KB").arg(loc.toString(bytes / 1024.0, 'f', 1)));
        else if (bytes < qint64(1024) * 1024 * 1024)
            mp_size->setText(tr("%1 MB").arg(loc.toString(bytes / (1024.0 * 1024), 'f', 2)));
        else
            mp_size->setText(tr("%1 GB").arg(loc.toString(bytes / (1024.0*1024*1024), 'f', 2)));
    }

    // Modified date
    mp_modified->setText(QLocale().toString(info.lastModified(), QLocale::ShortFormat));

    // Full path (elide in the middle if very long)
    mp_path->setText(info.absoluteFilePath());
}

void TeDetailFileInfoSection::clear()
{
    mp_name->clear();
    mp_type->clear();
    mp_size->clear();
    mp_modified->clear();
    mp_path->clear();
}

QString TeDetailFileInfoSection::title() const
{
    return tr("File Information");
}
