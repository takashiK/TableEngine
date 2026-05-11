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

#include "TeFontDialog.h"

#include <QFontComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>

#include <QColorDialog>
#include <QDialogButtonBox>

#include <QHBoxLayout>
#include <QVBoxLayout>

TeFontDialog::TeFontDialog(QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *fontLayout = new QHBoxLayout();
    fontLayout->addWidget(new QLabel(tr("Font:"), this));
    mp_fontComboBox = new QFontComboBox(this);
    fontLayout->addWidget(mp_fontComboBox);
    connect(mp_fontComboBox, &QFontComboBox::currentFontChanged, this, [this](const QFont & /*font*/) {
        updatePreview();
    });

    fontLayout->addWidget(new QLabel(tr("Size:"), this));
    mp_fontSize = new QSpinBox(this);
    mp_fontSize->setRange(1, 200);
    connect(mp_fontSize, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int /*value*/) {
        updatePreview();
    });
    fontLayout->addWidget(mp_fontSize);
    mainLayout->addLayout(fontLayout);

    QHBoxLayout *optionLayout = new QHBoxLayout();    
    optionLayout->addWidget(new QLabel(tr("Color:"), this));
    mp_color = new QPushButton();
    mp_color->setFixedWidth(50);
    connect(mp_color, &QPushButton::clicked, [this](bool /*checked*/) {
        QColor c = QColorDialog::getColor(color(), this, tr("Select Font Color"));
        if (c.isValid()) {
            setColor(c);
        }
    });
    optionLayout->addWidget(mp_color);

    optionLayout->addWidget(new QLabel(tr("Background Color:"), this));
    mp_bgColor = new QPushButton();
    mp_bgColor->setFixedWidth(50);
    connect(mp_bgColor, &QPushButton::clicked, [this](bool /*checked*/) {
        QColor c = QColorDialog::getColor(backgroundColor(), this, tr("Select Background Color"));
        if (c.isValid()) {
            setBackgroundColor(c);
        }
    });
    optionLayout->addWidget(mp_bgColor);
    optionLayout->addStretch();

    mainLayout->addLayout(optionLayout);

    mp_preview = new QLineEdit(this);
    //set sapmple text for preview
    mp_preview->setText(tr("Sample Text"));
    mainLayout->addWidget(mp_preview);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(buttonBox);

    updatePreview();
}

TeFontDialog::~TeFontDialog()
{
}

QFont TeFontDialog::font() const
{
    QFont font = mp_fontComboBox->currentFont();
    font.setPointSize(mp_fontSize->value());
    return font;
}

void TeFontDialog::setFont(const QFont &font)
{
    mp_fontComboBox->setCurrentFont(font);
    mp_fontSize->setValue(font.pointSize());
    updatePreview();
}

QColor TeFontDialog::color() const
{
    QColor color = mp_color->palette().color(QPalette::Button);
    return color;
}

void TeFontDialog::setColor(const QColor &color)
{
    //set QPalette::Button color to the given color
    QPalette pal = mp_color->palette();
    pal.setColor(QPalette::Button, color);
    mp_color->setPalette(pal);
    updatePreview();
}

QColor TeFontDialog::backgroundColor() const
{
    QColor color = mp_bgColor->palette().color(QPalette::Button);
    return color;
}

void TeFontDialog::setBackgroundColor(const QColor &color)
{
    //set QPalette::Button color to the given color
    QPalette pal = mp_bgColor->palette();
    pal.setColor(QPalette::Button, color);
    mp_bgColor->setPalette(pal);
    updatePreview();
}

void TeFontDialog::updatePreview()
{
    mp_preview->setFont(font());
    mp_preview->setStyleSheet(QString("color: %1; background-color: %2")
                              .arg(color().name())
                              .arg(backgroundColor().name()));
}
