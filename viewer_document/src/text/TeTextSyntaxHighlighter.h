#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>

#include "TeTextSyntax.h"

class TeTextSyntaxHighlighter :
    public QSyntaxHighlighter
{
    Q_OBJECT

public:
    TeTextSyntaxHighlighter(QTextDocument* parent = 0);
    void setTextSyntax(const TeTextSyntax& syntax) { m_syntax = syntax; }

protected:
    void highlightBlock(const QString& text) override;

private:
    TeTextSyntax m_syntax;

    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    struct RegionHighlightRule
    {
        QRegularExpression start_pattern;
        QRegularExpression end_pattern;
        QTextCharFormat format;
    };
    QVector<HighlightingRule> keywordRules;
    QVector<RegionHighlightRule> regionRules;
};

