#pragma once

#include "TeDocumentView.h"

class TeTextView :
    public TeDocumentView
{
    Q_OBJECT

public:
    TeTextView(QWidget* parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    QWidget* mp_lineNumberArea;
};

