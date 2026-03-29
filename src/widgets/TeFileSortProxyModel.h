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
#include <QSize>
#include "TeTypes.h"

class TeImageLoader;

class TeFileSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum Role {
        FilePixmap = Qt::UserRole + 50,
    };
    Q_ENUM(Role)

    explicit TeFileSortProxyModel(QObject* parent = nullptr);
    virtual ~TeFileSortProxyModel();

    void setSortType(TeTypes::OrderType type);
    TeTypes::OrderType sortType() const;

    void setPixmapSize(const QSize& size);
    QSize pixmapSize() const;

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void setSourceModel(QAbstractItemModel* sourceModel) override;

protected:
    virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
    static bool stringLessThan(const QString &left, const QString &right, Qt::CaseSensitivity cs, bool isLocaleAware);

private slots:
    void onImageReady(const QString& filePath);
    void onSourceRootPathChanged();

private:
    TeTypes::OrderType m_sortType;
    QSize m_pixmapSize;
    TeImageLoader* mp_imageLoader;
};
