#pragma once

#include <QMainWindow>

class TeDocumentView;
class TeSyntaxHighlighter;

class TeDocViewer :
    public QMainWindow
{
    Q_OBJECT

public:
    TeDocViewer(QWidget* parent = 0);

private:
    void setupEditor();

    TeDocumentView* mp_view;
    TeSyntaxHighlighter* mp_highlighter;

};

