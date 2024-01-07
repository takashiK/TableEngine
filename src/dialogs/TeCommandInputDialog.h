#pragma once

#include <QDialog>

class QLineEdit;
class QCheckBox;

class TeCommandInputDialog : public QDialog
{
	Q_OBJECT

public:
	TeCommandInputDialog(QWidget *parent = nullptr);
	~TeCommandInputDialog();

	QString command() const;
	bool shell() const;
	bool output() const;

private:
	QLineEdit* mp_command;
	QCheckBox* mp_shell;
	QCheckBox* mp_output;
};
