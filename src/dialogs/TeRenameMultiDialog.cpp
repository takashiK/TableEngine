#include "TeRenameMultiDialog.h"

#include <QLineEdit>
#include <QButtonGroup>
#include <QSpinBox>
#include <QLabel>
#include <QRadioButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

TeRenameMultiDialog::TeRenameMultiDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* pVLayout = new QVBoxLayout();
	QGridLayout* pGrid = new QGridLayout();
	QHBoxLayout* pHLayout;

	//Sample
	pGrid->addWidget(new QLabel(tr("Sample")),0,0);
	mp_sample = new QLineEdit();
	mp_sample->setReadOnly(true);
	mp_sample->setFocusPolicy(Qt::ClickFocus);
	pGrid->addWidget(mp_sample,0,1);

	//Base name
	pGrid->addWidget(new QLabel(tr("Base name")),1,0);
	mp_baseName = new QLineEdit();
	connect(mp_baseName, &QLineEdit::textChanged, [this](const QString&) {updateSample(); });
	pGrid->addWidget(mp_baseName, 1, 1);

	//Extention
	pGrid->addWidget(new QLabel(tr("Extention")),2,0);
	mp_extention = new QLineEdit();
	connect(mp_extention, &QLineEdit::textChanged, [this](const QString&) {updateSample(); });
	pGrid->addWidget(mp_extention, 2, 1);

	//Digits & Start
	pGrid->addWidget(new QLabel(tr("Digits")),3,0);
	pHLayout = new QHBoxLayout();
	mp_digits = new QSpinBox();
	mp_digits->setRange(1, 4);
	mp_digits->setValue(3);
	connect(mp_digits, &QSpinBox::valueChanged, [this](int) {updateSample(); });
	pHLayout->addWidget(mp_digits,1);
	pHLayout->addSpacing(10);
	pHLayout->addWidget(new QLabel(tr("Start")));
	mp_startNum = new QSpinBox();
	mp_startNum->setRange(0, 9999);
	mp_startNum->setValue(0);
	connect(mp_startNum, &QSpinBox::valueChanged, [this](int) {updateSample(); });
	pHLayout->addWidget(mp_startNum,1);
	pHLayout->addStretch();
	pGrid->addLayout(pHLayout, 3, 1);

	//Order
	pGrid->addWidget(new QLabel(tr("Order")),4,0);
	mp_order = new QButtonGroup();
	mp_order->addButton(new QRadioButton(tr("Filename")),0);
	mp_order->addButton(new QRadioButton(tr("Datetime")),1);
	mp_order->button(0)->setChecked(true);
	pHLayout = new QHBoxLayout();
	pHLayout->addWidget(mp_order->button(0));
	pHLayout->addWidget(mp_order->button(1));
	pHLayout->addStretch();
	pGrid->addLayout(pHLayout, 4, 1);

	pVLayout->addLayout(pGrid);

	//Standerd Buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	pVLayout->addWidget(buttonBox);



	setLayout(pVLayout);
}

TeRenameMultiDialog::~TeRenameMultiDialog()
{}

QString TeRenameMultiDialog::baseName() const
{
	return mp_baseName->text();
}

QString TeRenameMultiDialog::extention() const
{
	return mp_extention->text();
}

int TeRenameMultiDialog::digits() const
{
	return mp_digits->value();
}

int TeRenameMultiDialog::startNum() const
{
	return mp_startNum->value();
}

int TeRenameMultiDialog::order() const
{
	return mp_order->checkedId();
}

void TeRenameMultiDialog::updateSample()
{
	QString sample = mp_baseName->text();
	sample += QString("%1").arg(mp_startNum->value(), mp_digits->value(), 10, QChar('0'));
	sample += mp_extention->text();
	mp_sample->setText(sample);
}

