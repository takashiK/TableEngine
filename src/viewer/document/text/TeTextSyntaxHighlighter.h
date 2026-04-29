#pragma once

#include <QSyntaxHighlighter>
#include <QRegularExpression>

#include "TeTextSyntax.h"

/**
 * @file TeTextSyntaxHighlighter.h
 * @brief Declaration of TeTextSyntaxHighlighter.
 * @ingroup viewer
 */


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
};

