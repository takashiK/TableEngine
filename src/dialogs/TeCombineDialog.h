#pragma once

#include <QDialog>
#include <QStringList>
#include <QSet>

class QLineEdit;
class QListView;
class QCheckBox;

class TeCombineDialog : public QDialog
{
	Q_OBJECT

public:
	TeCombineDialog(QWidget *parent = nullptr);
	~TeCombineDialog();

	void setBasePath(const QString& path);
	QString outputFile() const;

	QStringList inputFiles() const;
	void setInputFiles(const QStringList& files);
	void setExtendSelection(bool extend);

protected:
	void updateFileList();

private:
	QLineEdit* mp_outputFile;
	QLineEdit* mp_combineSize;
	QListView* mp_fileList;
	QCheckBox* mp_extendSelection;

	QSet<QString> m_inputFiles;
	QSet<QString> m_baseNames;
	QString m_basePath;
};
