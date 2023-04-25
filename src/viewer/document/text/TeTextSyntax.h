#pragma once
#include <qobject.h>
#include <QList>
#include <QString>
#include <QTextCharFormat>
#include <QRegularExpression>

class TeTextSyntax
{
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

	void clear();

	const QList<SyntaxKeywords>& keywords() const { return m_keywords; }
	const QList<SyntaxRegex>& regex_keywords() { update_keywords(); return m_keywordRegexes; };
	const QList<SyntaxRegex>& regexes() const { return m_regexes; }
	const QList<SyntaxRegion>& regions() const { return m_regions; }

	int addSyntaxKeywords(const SyntaxKeywords& keyword);
	int addSyntaxRegex(const SyntaxRegex& regex);
	int addSyntaxRegion(const SyntaxRegion& region);

	bool removeSyntaxKeywords(int index);
	bool removeSyntaxRegex(int index);
	bool removeSyntaxRegion(int index);

private:
	void update_keywords();

	QList<SyntaxKeywords> m_keywords;
	bool m_update_keywords;
	QList<SyntaxRegex> m_keywordRegexes;
	QList<SyntaxRegex> m_regexes;
	QList<SyntaxRegion> m_regions;
};

