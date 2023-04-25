#pragma once

#include <QPlainTextEdit>

class TeLineNumberArea;

class TeTextView :
    public QPlainTextEdit
{
    Q_OBJECT

    friend TeLineNumberArea;
public:
    TeTextView(QWidget* parent = nullptr);
    bool isLineNumberVisible() const;
    void setLineNumberVisible(bool visible);
    int tabStopWidth() const;
    void setTabStopWidth(int tabStop);

signals:
    void tabStopWidthChanged(int tabStop);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();

private slots:
    void updateLineNumberAreaWidth();
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);

private:
    QWidget* mp_lineNumberArea;
    bool m_lineNumberVisible;
    int m_tabStop;
};

