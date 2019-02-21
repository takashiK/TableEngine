#pragma once

#include <QDialog>

class QButtonGroup;
class QLineEdit;

class TeAskCreationModeDialog : public QDialog
{
	Q_OBJECT

public:
	enum CreateMode{
		MODE_CREATE_FOLDER = 0x01,
		MODE_RENAME_FILE   = 0x02,
	};
	Q_ENUM(CreateMode)
public:
	TeAskCreationModeDialog(QWidget *parent);
	virtual  ~TeAskCreationModeDialog();

	void setModeEnabled(CreateMode mode, bool flag);

	void setCreateMode(CreateMode mode);
	CreateMode createMode();

	void setTargetPath(const QString& path);

private:
	QButtonGroup* mp_createMode;
	QLineEdit*    mp_path;
};
