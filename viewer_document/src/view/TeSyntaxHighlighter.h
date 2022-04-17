#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class TeSyntaxHighlighter :
    public QSyntaxHighlighter
{
    Q_OBJECT

public:
    TeSyntaxHighlighter(QTextDocument* parent = 0);

protected:
    void highlightBlock(const QString& text) override;

private:
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

