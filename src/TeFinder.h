#pragma once

#include <QObject>
#include <QString>
#include <QRegularExpression>
#include <QDateTime>
#include <QList>

#include "TeFileInfo.h"
#include "widgets/TeFolderView.h"

class TeFinder : public QObject
{
	Q_OBJECT

public:
	enum TeFinderType
	{
		TeFinderType_Invalid,
		TeFinderType_ByName,
		TeFinderType_BySize,
		TeFinderType_ByDate
	};

	TeFinder(QObject* parent);
	virtual ~TeFinder();
	virtual bool isValid() const = 0;

	void setRelatedView(TeFolderView* view);
	TeFolderView* relatedView() const;

	QString targetPath() const;
	TeFinderType type() const;
	QRegularExpression regExp() const;
	qsizetype filesize() const;
	bool lessthan() const;
	QDateTime date() const;

	void clear();
	const QList<TeFileInfo>& findedInfos() const;

	void cancel();
	void findByName(const QString& path, const QRegularExpression& regExp);
	void findBySize(const QString& path, qsizetype filesize, bool lessthan=true);
	void findByDate(const QString& path, const QDateTime& date, bool lessthan=true);

signals:
	void found(const QList<TeFileInfo>& fileInfos);
	void stepFinished();
	void finished();


protected slots:
	virtual void findStep() = 0;

protected:
	QList<TeFileInfo> m_fileInfos;
	QList<QString> m_targetPaths;

private:
	QString m_targetPath;
	bool m_lessThan;
	TeFinderType m_type;
	QRegularExpression m_regExp;
	qsizetype m_filesize;
	QDateTime m_date;

	TeFolderView* mp_relatedView;
};