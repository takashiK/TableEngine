#include "TeSyntaxHighlighter.h"

#include <QDebug>

#if 0
    Highlight patterns
    - condition                : ifdef / elsif / else / endif pattern.  ** not support. because it need analyze header chain **
    - multi line commnet       : /* */start pattern to end pattern. not nested.
    - single line comment      : // start pattern to end of line.
    - single string literal    : " " start and end bracket are same pattern.
    - keyword : same as matched keyword region.



#endif

static const int region_mask    = 0x0000FFFF;

TeSyntaxHighlighter::TeSyntaxHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    QTextCharFormat keywordFormat;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
        QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"),
        QStringLiteral("\\blong\\b"), QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bshort\\b"), QStringLiteral("\\bsignals\\b"), QStringLiteral("\\bsigned\\b"),
        QStringLiteral("\\bslots\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bstruct\\b"),
        QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"),
        QStringLiteral("\\bunion\\b"), QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\bvirtual\\b"),
        QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bbool\\b")
    };
    for (const QString& pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        keywordRules.append(rule);
    }

    QTextCharFormat classFormat;
    classFormat.setFontWeight(QFont::Bold);
    classFormat.setForeground(Qt::darkMagenta);
    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    rule.format = classFormat;
    keywordRules.append(rule);

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    keywordRules.append(rule);

    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    keywordRules.append(rule);

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(Qt::red);
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    keywordRules.append(rule);

    QTextCharFormat multiLineCommentFormat;
    RegionHighlightRule regionRule;
    multiLineCommentFormat.setForeground(Qt::red);

    regionRule.start_pattern = QRegularExpression(QStringLiteral("/\\*"));
    regionRule.end_pattern = QRegularExpression(QStringLiteral("\\*/"));
    regionRules.append(regionRule);
}

void TeSyntaxHighlighter::highlightBlock(const QString& text)
{
    qDebug() << text << "\n";

    // regex pattern base format
    for (const HighlightingRule& rule : qAsConst(keywordRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }

    // clear multi line format state
    setCurrentBlockState(0);

    // comment block detector
    int priority = 0;
    int startIndex = 0;
    if ((previousBlockState() & region_mask) == 0)
        startIndex = text.indexOf(regionRules[priority].start_pattern);

    while (startIndex >= 0) {
        QRegularExpressionMatch match = regionRules[priority].end_pattern.match(text, startIndex);
        int endIndex = match.capturedStart();
        int commentLength = 0;
        if (endIndex == -1) {
            setCurrentBlockState(1);
            commentLength = text.length() - startIndex;
        }
        else {
            commentLength = endIndex - startIndex
                + match.capturedLength();
        }
        setFormat(startIndex, commentLength, regionRules[priority].format);
        startIndex = text.indexOf(regionRules[priority].start_pattern, startIndex + commentLength);
    }
}
