#pragma once

#include <QDialog>
#include "utils/TeToolCommand.h"

/**
 * @file TeCommandInputDialog.h
 * @brief Declaration of TeCommandInputDialog.
 * @ingroup dialogs
 */


class QLineEdit;
class QCheckBox;
class QComboBox;

class TeCommandInputDialog : public QDialog
{
	Q_OBJECT

public:
	TeCommandInputDialog(QWidget *parent = nullptr);
	~TeCommandInputDialog();

	QString command() const;
	bool shell() const;
	TeToolCommand::OUTPUT_MODE output() const;

private:
	QLineEdit* mp_command;
	QCheckBox* mp_shell;
	QComboBox* mp_output;
};
