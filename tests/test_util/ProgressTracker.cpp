#include "ProgressTracker.h"

ProgressTracker::ProgressTracker(QObject *parent)
	: QObject(parent)
{
	clear();
}

ProgressTracker::~ProgressTracker()
{
}

void ProgressTracker::clear()
{
	count = 0;
	maxvalue = 0;
	progress = 0;
	strList.clear();
}

void ProgressTracker::setMaxValue(int value)
{
	maxvalue = value;
}

void ProgressTracker::setProgress(int value)
{
	count++;
	progress = value;
}

void ProgressTracker::setText(const QString& value)
{
	strList.append(value);
}
