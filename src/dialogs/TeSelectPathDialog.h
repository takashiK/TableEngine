#pragma once

#include <QDialog>

class QLineEdit;
class TeFileTreeView;

class TeSelectPathDialog : public QDialog
{
	Q_OBJECT

public:
	TeSelectPathDialog(QWidget *parent);
	virtual  ~TeSelectPathDialog();

	QString targetPath();

public slots:
	void setTargetPath(const QString& path);

private:
	QLineEdit*  mp_edit;
	TeFileTreeView* mp_tree;
};
