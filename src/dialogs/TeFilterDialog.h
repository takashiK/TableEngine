#pragma once

#include <QDialog>

class QCheckBox;
class QLineEdit;

class TeFilterDialog : public QDialog
{
	Q_OBJECT

public:
	TeFilterDialog(QWidget *parent = nullptr);
	~TeFilterDialog();

	QString filter() const;
	bool fileOnly() const;
	bool caseSensitive() const;
	bool regexp() const;

private:
	QLineEdit *mp_filter;
	QCheckBox* mp_fileOnly;
	QCheckBox *mp_caseSensitive;
	QCheckBox *mp_regexp;
};
