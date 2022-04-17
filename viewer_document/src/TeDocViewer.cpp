#include "TeDocViewer.h"

#include "view/TeTextView.h"
#include "view/TeSyntaxHighlighter.h"

TeDocViewer::TeDocViewer(QWidget* parent)
    : QMainWindow(parent)
{
    setupEditor();

    setCentralWidget(mp_view);
    setWindowTitle(tr("Syntax Highlighter"));
}

void TeDocViewer::setupEditor()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    mp_view = new TeTextView();
    mp_view->setFont(font);

    mp_highlighter = new TeSyntaxHighlighter(mp_view->document());

    QFile file("mainwindow.h");
    if (file.open(QFile::ReadOnly | QFile::Text))
        mp_view->setPlainText(file.readAll());
}

