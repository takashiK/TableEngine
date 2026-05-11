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

#include <QDialog>

/**
 * @file TeFontDialog.h
 * @brief Declaration of TeFontDialog.
 * @ingroup dialogs
 */

class QFontComboBox;
class QSpinBox;
class QCheckBox;
class QLabel;
class QPushButton;
class QLineEdit;

class TeFontDialog : public QDialog
{
    Q_OBJECT

public:
    TeFontDialog(QWidget *parent = nullptr);
    ~TeFontDialog();

    QFont font() const;
    void setFont(const QFont &font);
    QColor color() const;
    void setColor(const QColor &color);
    QColor backgroundColor() const;
    void setBackgroundColor(const QColor &color);

private:
    void updatePreview();

private:
    QFontComboBox* mp_fontComboBox;
    QSpinBox* mp_fontSize;
    QPushButton* mp_color;
    QPushButton* mp_bgColor;
    QLineEdit* mp_preview;
};