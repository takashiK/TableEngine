#include "TeTextView.h"

#include <QPainter>
#include <QTextBlock>

class TeLineNumberArea :
    public QWidget
{
public:
    TeLineNumberArea(TeTextView* view) : QWidget(view), textView(view)
    {}

    QSize sizeHint() const override
    {
        return QSize(textView->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        textView->lineNumberAreaPaintEvent(event);
    }

private:
    TeTextView* textView;
};

TeTextView::TeTextView(QWidget* parent) : TeDocumentView(parent)
{
    mp_lineNumberArea = new TeLineNumberArea(this);

    connect(this, &TeTextView::blockCountChanged, this, &TeTextView::updateLineNumberAreaWidth);
    connect(this, &TeTextView::updateRequest, this, &TeTextView::updateLineNumberArea);
    connect(this, &TeTextView::cursorPositionChanged, this, &TeTextView::highlightCurrentLine);

    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int TeTextView::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;

    return space;
}

void TeTextView::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void TeTextView::updateLineNumberArea(const QRect& rect, int dy)
{
    if (dy)
        mp_lineNumberArea->scroll(0, dy);
    else
        mp_lineNumberArea->update(0, rect.y(), mp_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void TeTextView::resizeEvent(QResizeEvent* e)
{
    QPlainTextEdit::resizeEvent(e);

    QRect cr = contentsRect();
    mp_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void TeTextView::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;

        QColor lineColor = QColor(Qt::yellow).lighter(160);

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }

    setExtraSelections(extraSelections);
}

void TeTextView::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(mp_lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, mp_lineNumberArea->width(), fontMetrics().height(),
                Qt::AlignRight, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

