#pragma once

#include <QMainWindow>

class TeDocumentView;
class TeTextSyntaxHighlighter;

class TeDocViewer :
    public QMainWindow
{
    Q_OBJECT

public:
    TeDocViewer(QWidget* parent = 0);
    void open(QString filepath);

private:
    void setupViewer();

    TeDocumentView* mp_view;
    TeTextSyntaxHighlighter* mp_highlighter;

};

