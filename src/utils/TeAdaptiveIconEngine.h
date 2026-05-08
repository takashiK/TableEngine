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

/**
 * @file TeAdaptiveIconEngine.h
 * @brief QIconEngine that tints icons to light grey in dark mode.
 * @ingroup utility
 *
 * Pass an instance to the QIcon(QIconEngine*) constructor so that every
 * paint/pixmap request automatically returns the correctly coloured version
 * for the current OS colour scheme.
 */

#include <QIconEngine>
#include <QString>
#include <QPixmap>

/**
 * @class TeAdaptiveIconEngine
 * @brief Icon engine that adapts icon colour to the OS dark/light mode.
 *
 * In light mode the original resource pixmap is returned unchanged.
 * In dark mode the pixmap is tinted to @c darkColor (default: #c8c8c8)
 * using the original alpha channel as a mask so only the shape is
 * the alpha channel is preserved so tone gradations in the original are retained.
 */
class TeAdaptiveIconEngine : public QIconEngine
{
public:
    /**
     * @brief Constructs the engine for the given Qt resource path.
     * @param resourcePath  Path to the icon resource, e.g. ":/TableEngine/newFile.png".
     */
    explicit TeAdaptiveIconEngine(const QString& resourcePath);

    void    paint(QPainter* painter, const QRect& rect,
                  QIcon::Mode mode, QIcon::State state) override;

    QPixmap pixmap(const QSize& size,
                   QIcon::Mode mode, QIcon::State state) override;

    QIconEngine* clone() const override;

private:
    /** @brief Returns @p src with RGB channels inverted; alpha channel is unchanged. */
    static QPixmap inverted(const QPixmap& src);

    QString m_path;
    QColor  m_darkColor;
};
