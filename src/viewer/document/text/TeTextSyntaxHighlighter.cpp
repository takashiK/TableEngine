#include "TeTextSyntaxHighlighter.h"
#include "TeTextSyntaxLoader.h"

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
}

void TeTextSyntaxHighlighter::highlightBlock(const QString& text)
{
    //qDebug() << text << "\n";

    // keywords
    for (const auto& syntax : m_syntax.regex_keywords()) {
        QRegularExpressionMatchIterator matchIterator = syntax.regex.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), syntax.format);
        }
    }

    // regex
    for (const auto& syntax : m_syntax.regexes()) {
        QRegularExpressionMatchIterator matchIterator = syntax.regex.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), syntax.format);
            
//            qDebug() << "match:" << syntax.regex.pattern() << "\n";
        }
    }

    // clear multi line format state
    setCurrentBlockState(0);

    // comment block detector
    int startIndex = 0;
    int previous = previousBlockState();
    while (1) {
        int current = m_syntax.regions().length();
        for (; current > 0 && current > previous; current--) {
            QRegularExpressionMatch match = m_syntax.regions().at(current - 1).startRegex.match(text, startIndex);
            if (match.hasMatch()) {
                startIndex = match.capturedStart();
                break;
            }
        }
        if (current > 0) {
            // found multiline start
            const auto& region = m_syntax.regions().at(current - 1);
            QRegularExpressionMatch match = region.endRegex.match(text, startIndex);
            int commentLength = 0;
            if (match.hasMatch()) {
                // end of current multi line state.
                previous = 0;
                int endIndex = match.capturedStart();
                commentLength = endIndex - startIndex
                    + match.capturedLength();
                setFormat(startIndex, commentLength, region.format);
                // continue search muliti line statement after endIndex.
                startIndex = endIndex;
            }
            else {
                //couldn't found end statemate for current sate.
                setCurrentBlockState(current);
                commentLength = text.length() - startIndex;
                setFormat(startIndex, commentLength, region.format);
                break;
            }
        }
        else {
            // no more multi line statement.
            setCurrentBlockState(0);
            break;
        }
    }

}
