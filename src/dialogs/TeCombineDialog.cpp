#include "TeCombineDialog.h"

#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QListView>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDialogButtonBox>

#include <QStandardItemModel>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QFileDialog>

namespace {
	QList<QFileInfo> getTargetFiles(const QSet<QString>& files, bool extend)
	{
		QSet<QString> targetFileSet;
		QSet<QString> baseNames;

		//setup target files & base names
		for (const QString& file : files)
		{
			QFileInfo fileInfo(file);
			if (fileInfo.exists() && fileInfo.isFile()) {
				targetFileSet.insert(fileInfo.absoluteFilePath());
				baseNames.insert(fileInfo.absolutePath() + "/" + fileInfo.completeBaseName());
			}
		}

		//extend selection by baseNames
		//only accespt style "basename.[0-9]+"
		if (extend) {
			QRegularExpression re("^[0-9]+$");
			for (const QString& baseName : baseNames) {
				QFileInfo base(baseName);
				QDir dir(base.absolutePath());
				QStringList nameFilters;
				nameFilters << base.fileName() + ".*";
				QStringList entries = dir.entryList(nameFilters, QDir::Files);
				for (const QString& entry : entries) {
					QFileInfo fileInfo(dir.absolutePath() + "/" + entry);
					if (fileInfo.completeBaseName() == base.fileName()
						&& re.match(fileInfo.suffix()).hasMatch()) {
						targetFileSet.insert(fileInfo.absoluteFilePath());
					}
				}
			}
		}

		QList<QFileInfo> targetFiles(targetFileSet.begin(),targetFileSet.end());
		std::sort(targetFiles.begin(), targetFiles.end(), [](const QFileInfo& a, const QFileInfo& b) {
			return a.fileName() < b.fileName();
		});


		return targetFiles;
	}
}

TeCombineDialog::TeCombineDialog(QWidget *parent)
	: QDialog(parent)
{
	QVBoxLayout* pVLayout = new QVBoxLayout();
	QGridLayout* pGrid = new QGridLayout();

	int gridRow = 0;

	//Output file
	pGrid->addWidget(new QLabel(tr("Output file")), gridRow, 0);
	QHBoxLayout* pHLayout = new QHBoxLayout();
	mp_outputFile = new QLineEdit();
	pHLayout->addWidget(mp_outputFile);
	QPushButton* pButton = new QPushButton(tr("..."));
		connect(pButton, &QPushButton::clicked, [this]() {
			QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), m_basePath);
			if (!fileName.isEmpty()) {
				QDir dir(m_basePath);
				QString relativePath = dir.relativeFilePath(fileName);
				if (relativePath.startsWith("..")) {
					mp_outputFile->setText(fileName);
				}
				else {
					mp_outputFile->setText(relativePath);
				}
			}
		});
	pHLayout->addWidget(pButton);
	pGrid->addLayout(pHLayout, gridRow, 1);

	//Extend selection
	mp_extendSelection = new QCheckBox(tr("Extend selection"));
	connect(mp_extendSelection, &QCheckBox::stateChanged, [this](int state) { updateFileList();});
	pGrid->addWidget(mp_extendSelection, ++gridRow, 1);

	//Input file list
	pGrid->addWidget(new QLabel(tr("Input files")), ++gridRow, 0);
	mp_fileList = new QListView();
	mp_fileList->setModel(new QStandardItemModel());
	pGrid->addWidget(mp_fileList, gridRow, 1);

	//Combine size
	pGrid->addWidget(new QLabel(tr("Combine size")), ++gridRow, 0);
	mp_combineSize = new QLineEdit();
	mp_combineSize->setFocusPolicy(Qt::ClickFocus);
	pGrid->addWidget(mp_combineSize, gridRow, 1);

	pVLayout->addLayout(pGrid);

	//Standerd Buttons
	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	buttonBox->setCenterButtons(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	pVLayout->addWidget(buttonBox);



	setLayout(pVLayout);
}

TeCombineDialog::~TeCombineDialog()
{}

void TeCombineDialog::setBasePath(const QString& path)
{
	m_basePath = path;
}

QString TeCombineDialog::outputFile() const
{
	return mp_outputFile->text();
}

QStringList TeCombineDialog::inputFiles() const
{
	int count = mp_fileList->model()->rowCount();
	QStringList files;
	for (int i = 0; i < count; ++i) {
		QStandardItem* item = static_cast<QStandardItemModel*>(mp_fileList->model())->item(i);
		files << item->accessibleText();
	}
	return files;
}

void TeCombineDialog::setInputFiles(const QStringList& files)
{

	m_inputFiles = QSet<QString>(files.begin(), files.end());

	updateFileList();
}

void TeCombineDialog::setExtendSelection(bool extend)
{
	mp_extendSelection->setChecked(extend);

	updateFileList();
}

void TeCombineDialog::updateFileList()
{
	QStandardItemModel* model = qobject_cast<QStandardItemModel*>(mp_fileList->model());
	QList<QFileInfo> fileInfos = getTargetFiles(m_inputFiles, mp_extendSelection->isChecked());

	model->clear();
	qsizetype size = 0;

	if (mp_outputFile->text().isEmpty()) {
		mp_outputFile->setText(fileInfos.first().completeBaseName());
	}

	for (const QFileInfo& info : fileInfos) {
		size += info.size();
		QStandardItem* item = new QStandardItem(info.fileName());
		item->setAccessibleText(info.absoluteFilePath());
		model->appendRow(item);
	}

	QString suffix = " Byte";
	if (size > 1024) {
		size /= 1024;
		suffix = " KB";
		if(size > 1024) {
			size /= 1024;
			suffix = " MB";
			if (size > 1024) {
				size /= 1024;
				suffix = " GB";
			}
		}
	}
	mp_combineSize->setText(QString::number(size) + suffix);
}
