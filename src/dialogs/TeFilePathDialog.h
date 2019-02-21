#pragma once

#include <QDialog>
class QComboBox;

class TeFilePathDialog : public QDialog
{
	Q_OBJECT

public:
	TeFilePathDialog(QWidget *parent = nullptr);
	virtual  ~TeFilePathDialog();

	void setCurrentPath(const QString& path);
	QString currentPath();

	void setTargetPath(const QString& path);
	QString targetPath();

//	void addWidget(QWidget *widget, int stretch = 0, Qt::Alignment alignment = Qt::Alignment());

protected:
	virtual bool eventFilter(QObject *obj, QEvent *event);

private:
	QComboBox* mp_combo;
	QString    m_currentPath;
};
