#pragma once
#include <qobject.h>
#include <QList>
#include <QString>
#include <QTextCharFormat>
#include <QRegularExpression>

class TeTextSyntax :
    public QObject
{
	Q_OBJECT
	
public:
	struct SyntaxKeywords
	{
		QTextCharFormat format;
		QStringList keywords;
	};

	struct SyntaxRegex
	{
		QTextCharFormat format;
		QRegularExpression regex;

	};

	struct SyntaxRegion
	{
		QTextCharFormat format;
		QRegularExpression startRegex;
		QRegularExpression endRegex;
	};

	TeTextSyntax();
	virtual ~TeTextSyntax();

	void load(QString path);
	void save(QString path);

	void clear();

	const QList<SyntaxKeywords>* keywords() { return &m_keywords; }
	const QList<SyntaxRegex>* regexes() { return &m_regexes; }
	const QList<SyntaxRegion>* regions() { return &m_regions; }

	int addSyntaxKeywords(const SyntaxKeywords& keyword);
	int addSyntaxRegex(const SyntaxRegex& regex);
	int addSyntaxRegion(const SyntaxRegion& region);

	bool removeSyntaxKeywords(int index);
	bool removeSyntaxRegex(int index);
	bool removeSyntaxRegion(int index);

private:
	QList<SyntaxKeywords> m_keywords;
	QList<SyntaxRegex> m_regexes;
	QList<SyntaxRegion> m_regions;
};

