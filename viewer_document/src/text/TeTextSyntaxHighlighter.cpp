#include "TeTextSyntaxHighlighter.h"

#include <QDebug>

#if 0
    Highlight patterns
    - multi line commnet       : /* */start pattern to end pattern. not nested. (Prioritical nest support)
    - single line comment      : // start pattern to end of line.  (Regx Base/overwrite)
    - single string literal    : " " start and end bracket are same pattern. (Regx Base/overwrite)
    - keyword : same as matched keyword region. (Regx Base/overwrite)



#endif

static const int region_mask    = 0x0000FFFF;

TeTextSyntaxHighlighter::TeTextSyntaxHighlighter(QTextDocument* parent)
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

    QTextCharFormat functionFormat;
    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+ *(?=\\()"));
    rule.format = functionFormat;
    keywordRules.append(rule);

    QTextCharFormat quotationFormat;
    quotationFormat.setForeground(Qt::darkRed);
    rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
    rule.format = quotationFormat;
    keywordRules.append(rule);

    QTextCharFormat singleLineCommentFormat;
    singleLineCommentFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    keywordRules.append(rule);

    QTextCharFormat multiLineCommentFormat;
    RegionHighlightRule regionRule;
    multiLineCommentFormat.setForeground(Qt::darkGreen);

    regionRule.start_pattern = QRegularExpression(QStringLiteral("/\\*"));
    regionRule.end_pattern = QRegularExpression(QStringLiteral("\\*/"));
    regionRule.format = multiLineCommentFormat;
    regionRules.append(regionRule);
}

void TeTextSyntaxHighlighter::highlightBlock(const QString& text)
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
