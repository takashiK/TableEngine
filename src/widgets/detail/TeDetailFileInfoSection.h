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

class QFormLayout;
class QLabel;

/**
 * @file TeDetailFileInfoSection.h
 * @brief Detail section that shows basic filesystem metadata for any file.
 * @ingroup widgets
 */

/**
 * @class TeDetailFileInfoSection
 * @brief Shows name, type, size, last-modified date and full path for any file.
 * @ingroup widgets
 *
 * @details Always returns true from canHandle(). Load is synchronous so no
 * ready() signal is emitted.
 *
 * @see TeDetailSection, TeDetailView
 */
class TeDetailFileInfoSection : public TeDetailSection
{
    Q_OBJECT

public:
    explicit TeDetailFileInfoSection(QWidget* parent = nullptr);

    bool    canHandle(const QFileInfo& info) const override;
    void    load(const QFileInfo& info) override;
    void    clear() override;
    QString title() const override;

private:
    QLabel* mp_name     = nullptr;
    QLabel* mp_type     = nullptr;
    QLabel* mp_size     = nullptr;
    QLabel* mp_modified = nullptr;
    QLabel* mp_path     = nullptr;
};
