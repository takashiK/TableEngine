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

protected:
    void textViewMode();
    void richViewMode();

private:
    void setupViewer();
    void setupMenu();

    TeDocumentView* mp_textView;
    TeTextSyntaxHighlighter* mp_textHighlighter;

};

