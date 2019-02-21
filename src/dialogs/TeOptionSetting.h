#pragma once

#include <QDialog>
#include <QHash>
#include <QVariant>

class TeOptionSetting : public QDialog
{
	Q_OBJECT

public: 

public:
	TeOptionSetting(QWidget *parent);
	virtual ~TeOptionSetting();
	
	void updateSettings();
	static void storeDefaultSettings();

public slots:
	void accept();

protected:
	QWidget* createPageStartup();
	QWidget* createPageFolder();
	QWidget* createPageView();
	QHash<QString, QVariant> m_option;
};
