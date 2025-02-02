#pragma once

#include <QDialog>

class QListView;
class QLabel;

class TePathListDialog : public QDialog
{
	Q_OBJECT

public:
	TePathListDialog(QWidget *parent = nullptr);
	~TePathListDialog();

	void setLabelText(const QString& text);

	void setPathList(const QStringList& pathList);
	QStringList getPathList() const;

protected slots:
	void onAdd(bool checked = false);
	void onRemove(bool checked = false);
	void onUp(bool checked = false);
	void onDown(bool checked = false);

private:
	QLabel* m_label;
	QListView* m_listView;
};
