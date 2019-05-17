#pragma once

#include <QObject>
#include <QString>

class ProgressTracker : public QObject
{
	Q_OBJECT

public:
	ProgressTracker(QObject* parent = nullptr);
	virtual ~ProgressTracker();

	void clear();

public slots:
	void setMaxValue(int value);
	void setProgress(int value);
	void setText(const QString& value);

public:
	int count;
	int maxvalue;
	int progress;
	QStringList strList;
};
