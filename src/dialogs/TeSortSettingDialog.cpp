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

#include "TeSortSettingDialog.h"
#include "TeSettings.h"

#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>

TeSortSettingDialog::TeSortSettingDialog(QWidget* parent) {
    setMinimumWidth(TeSettings::dialogMinimumWidth());
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // Sort Group
    mp_SortGroup = new QButtonGroup(this);
    QRadioButton* sortByName = new QRadioButton(tr("Name (&N)"), this);
    sortByName->setShortcut(QKeySequence(Qt::Key_N));
    QRadioButton* sortBySize = new QRadioButton(tr("Size (&S)"), this);
    sortBySize->setShortcut(QKeySequence(Qt::Key_S));
    QRadioButton* sortByType = new QRadioButton(tr("Extension (&X)"), this);
    sortByType->setShortcut(QKeySequence(Qt::Key_X));
    QRadioButton* sortByDate = new QRadioButton(tr("DateTime (&T)"), this);
    sortByDate->setShortcut(QKeySequence(Qt::Key_T));

    mp_SortGroup->addButton(sortByName, static_cast<int>(TeTypes::ORDER_NAME));
    mp_SortGroup->addButton(sortBySize, static_cast<int>(TeTypes::ORDER_SIZE));
    mp_SortGroup->addButton(sortByType, static_cast<int>(TeTypes::ORDER_EXTENSION));
    mp_SortGroup->addButton(sortByDate, static_cast<int>(TeTypes::ORDER_MODIFIED));

    QGridLayout* sortLayout = new QGridLayout();
    sortLayout->addWidget(sortByName, 0, 0);
    sortLayout->addWidget(sortBySize, 0, 1);
    sortLayout->addWidget(sortByType, 1, 0);
    sortLayout->addWidget(sortByDate, 1, 1);

    QGroupBox* sortGroupBox = new QGroupBox(tr("Sort By"), this);
    sortGroupBox->setLayout(sortLayout);
    mainLayout->addWidget(sortGroupBox);

    // Order Group
    mp_OrderGroup = new QButtonGroup(this);
    QRadioButton* orderAsc = new QRadioButton(tr("Ascending (&A)"), this);
    orderAsc->setShortcut(QKeySequence(Qt::Key_A));
    QRadioButton* orderDesc = new QRadioButton(tr("Descending (&D)"), this);
    orderDesc->setShortcut(QKeySequence(Qt::Key_D));

    mp_OrderGroup->addButton(orderAsc, OrderAscending);
    mp_OrderGroup->addButton(orderDesc, OrderDescending);
    QHBoxLayout* orderLayout = new QHBoxLayout();
    orderLayout->addWidget(orderAsc);
    orderLayout->addWidget(orderDesc);
    QGroupBox* orderGroupBox = new QGroupBox(tr("Order"), this);
    orderGroupBox->setLayout(orderLayout);
    mainLayout->addWidget(orderGroupBox);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttonBox->setCenterButtons(true);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TeSortSettingDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TeSortSettingDialog::reject);
    mainLayout->addWidget(buttonBox);
}

TeSortSettingDialog::~TeSortSettingDialog() {
}

TeTypes::OrderType TeSortSettingDialog::orderType() const {
    return static_cast<TeTypes::OrderType>(mp_SortGroup->checkedId());
}

void TeSortSettingDialog::setOrderType(TeTypes::OrderType type) {
    QAbstractButton* button = mp_SortGroup->button(static_cast<int>(type));
    if (button) {
        button->setChecked(true);
    }
}

bool TeSortSettingDialog::isDescending() const {
    return mp_OrderGroup->checkedId() == OrderDescending;
}

void TeSortSettingDialog::setDescending(bool descending) {
    QAbstractButton* button = mp_OrderGroup->button(descending ? OrderDescending : OrderAscending);
    if (button) {
        button->setChecked(true);
    }
}
