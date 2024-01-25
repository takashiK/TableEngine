#pragma once

#include <QDialog>

class QLineEdit;
class QButtonGroup;
class QSpinBox;

class TeRenameMultiDialog : public QDialog
{
	Q_OBJECT

public:
	enum Order {
		ORDER_FILENAME = 0,
		ORDER_DATETIME
	};

	TeRenameMultiDialog(QWidget *parent = nullptr);
	~TeRenameMultiDialog();

	QString baseName() const;
	QString extention() const;
	int digits() const;
	int startNum() const;
	int order() const;

protected slots:
	void updateSample();

private:
	QLineEdit* mp_baseName;
	QLineEdit* mp_extention;
	QLineEdit* mp_sample;
	QButtonGroup* mp_order;
	QSpinBox* mp_digits;
	QSpinBox* mp_startNum;
};
