#include "TeDivideDialog.h"

#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <QFileInfo>

TeDivideDialog::TeDivideDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* pVLayout = new QVBoxLayout();
	QGridLayout* pGrid = new QGridLayout();
	int gridRow = 0;

	//File path
	pGrid->addWidget(new QLabel(tr("File path")), gridRow, 0);
	mp_filename = new QLineEdit();
	mp_filename->setReadOnly(true);
	mp_filename->setFocusPolicy(Qt::ClickFocus);
	pGrid->addWidget(mp_filename, gridRow, 1);

	//Output path
	pGrid->addWidget(new QLabel(tr("Path")),++gridRow,0);
	mp_outputPath = new QLineEdit();
	pGrid->addWidget(mp_outputPath, gridRow,1);


	//Divide size
	pGrid->addWidget(new QLabel(tr("Divide size")), ++gridRow, 0);
	mp_divideSize = new QSpinBox();
	mp_divideSize->setRange(1, 1024);
	mp_divideSize->setValue(10);
	mp_divideSize->setSuffix(QObject::tr(" MB"));
	connect(mp_divideSize, &QSpinBox::valueChanged, [this](int) {updateCount(); });
	pGrid->addWidget(mp_divideSize, gridRow, 1);

	//Divide count
	pGrid->addWidget(new QLabel(tr("Divide count")),++gridRow,0);
	mp_fileCount = new QLineEdit();
	mp_fileCount->setReadOnly(true);
	mp_fileCount->setFocusPolicy(Qt::ClickFocus);
	pGrid->addWidget(mp_fileCount, gridRow, 1);

	pVLayout->addLayout(pGrid);

	//Standerd Buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	pVLayout->addWidget(buttonBox);



	setLayout(pVLayout);
}

TeDivideDialog::~TeDivideDialog()
{}

void TeDivideDialog::setFilepath(const QString & path)
{
	m_fileInfo.setFile(path);
	mp_filename->setText(m_fileInfo.fileName());
	updateCount();
}

QString TeDivideDialog::filePath() const
{
	return m_fileInfo.filePath();
}

QString TeDivideDialog::outPath() const
{
	return mp_outputPath->text();
}

qsizetype TeDivideDialog::size() const
{
	return mp_divideSize->value() * 1024 * 1024;
}

void TeDivideDialog::updateCount()
{
	qsizetype divsize = size();
	if (divsize <= 0) {
		mp_fileCount->setText(tr("Invalid size"));
		return;
	}

	qsizetype count = m_fileInfo.size() / divsize;
	if (m_fileInfo.size() % divsize != 0) {
		count++;
	}
	mp_fileCount->setText(QString::number(count));
}