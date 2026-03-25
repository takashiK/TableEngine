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

#include <QSortFilterProxyModel>
#include "TeTypes.h"

class TeFileSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit TeFileSortProxyModel(QObject* parent = nullptr);
    virtual ~TeFileSortProxyModel();

    void setSortType(TeTypes::OrderType type);
    TeTypes::OrderType sortType() const;

protected:
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    static bool stringLessThan(const QString &left, const QString &right, Qt::CaseSensitivity cs, bool isLocaleAware);

private:
    TeTypes::OrderType m_sortType;
};
