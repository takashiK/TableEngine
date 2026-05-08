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

#include "TeAdaptiveIconEngine.h"

#include <QPainter>
#include <QImage>
#include <QGuiApplication>
#include <QStyleHints>

TeAdaptiveIconEngine::TeAdaptiveIconEngine(const QString& resourcePath)
    : m_path(resourcePath)
{
}

QPixmap TeAdaptiveIconEngine::inverted(const QPixmap& src)
{
    QImage img = src.toImage().convertToFormat(QImage::Format_ARGB32);
    img.invertPixels(QImage::InvertRgb);  // RGB反転、アルファチャンネルは保持
    return QPixmap::fromImage(img);
}

QPixmap TeAdaptiveIconEngine::pixmap(const QSize& size, QIcon::Mode mode, QIcon::State state)
{
    Q_UNUSED(mode)
    Q_UNUSED(state)

    QPixmap px(m_path);
    if (!px.isNull() && size.isValid())
        px = px.scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (!px.isNull()
        && QGuiApplication::styleHints()->colorScheme() == Qt::ColorScheme::Dark)
    {
        return inverted(px);
    }

    return px;
}

void TeAdaptiveIconEngine::paint(QPainter* painter, const QRect& rect,
                                  QIcon::Mode mode, QIcon::State state)
{
    QPixmap px = pixmap(rect.size(), mode, state);
    if (!px.isNull())
        painter->drawPixmap(rect, px);
}

QIconEngine* TeAdaptiveIconEngine::clone() const
{
    return new TeAdaptiveIconEngine(m_path);
}
