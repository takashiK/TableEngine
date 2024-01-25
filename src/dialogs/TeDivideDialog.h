#pragma once

#include <QDialog>
#include <QFileInfo>

class QLineEdit;
class QSpinBox;

class TeDivideDialog : public QDialog
{
	Q_OBJECT

public:

	TeDivideDialog(QWidget *parent = nullptr);
	~TeDivideDialog();

	void setFilepath(const QString& path);
	QString filePath() const;
	QString outPath() const;
	qsizetype size() const;

protected slots:
	void updateCount();

private:
	QFileInfo m_fileInfo;
	QLineEdit* mp_filename;
	QLineEdit* mp_outputPath;
	QLineEdit* mp_fileCount;
	QSpinBox* mp_divideSize;
};
