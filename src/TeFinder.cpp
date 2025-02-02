#include "TeFinder.h"

TeFinder::TeFinder(QObject *parent)
	: QObject(parent)
{
	mp_relatedView = nullptr;
	m_filesize = 0;
	m_lessThan = true;
	m_type = TeFinderType_Invalid;
	connect(this, &TeFinder::stepFinished, this, &TeFinder::findStep, Qt::QueuedConnection);
}

TeFinder::~TeFinder()
{
	clear();
}

void TeFinder::setRelatedView(TeFolderView* view)
{
	mp_relatedView = view;
}

TeFolderView* TeFinder::relatedView() const
{
	return mp_relatedView;
}

QString TeFinder::targetPath() const
{
	return m_targetPath;
}

TeFinder::TeFinderType TeFinder::type() const
{
	return m_type;
}

QRegularExpression TeFinder::regExp() const
{
	return m_regExp;
}

qsizetype TeFinder::filesize() const
{
	return m_filesize;
}

bool TeFinder::lessthan() const
{
	return m_lessThan;
}

QDateTime TeFinder::date() const
{
	return m_date;
}

void TeFinder::clear()
{
	cancel();
	m_type = TeFinderType_Invalid;
	m_targetPath.clear();
	m_fileInfos.clear();
}

const QList<TeFileInfo>& TeFinder::findedInfos() const
{
	return m_fileInfos;
}

void TeFinder::cancel()
{
	m_targetPaths.clear();
}

void TeFinder::findByName(const QString& path, const QRegularExpression& regExp)
{
	clear();
	m_targetPath = path;
	m_type = TeFinderType_ByName;
	m_regExp = regExp;
	m_targetPaths.append(path);
	findStep();
}

void TeFinder::findBySize(const QString& path, qsizetype filesize, bool lessthan)
{
	clear();
	m_targetPath = path;
	m_type = TeFinderType_BySize;
	m_filesize = filesize;
	m_lessThan = lessthan;
	m_targetPaths.append(path);
	findStep();
}

void TeFinder::findByDate(const QString& path, const QDateTime& date, bool lessthan)
{
	clear();
	m_targetPath = path;
	m_type = TeFinderType_ByDate;
	m_date = date;
	m_lessThan = lessthan;
	m_targetPaths.append(path);
	findStep();
}
