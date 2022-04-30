#include "TeDocViewer.h"

#include "text/TeTextView.h"
#include "text/TeTextSyntaxHighlighter.h"

TeDocViewer::TeDocViewer(QWidget* parent)
    : QMainWindow(parent)
{
    setupViewer();

    setCentralWidget(mp_view);
    setWindowTitle(tr("Syntax Highlighter"));
}

void TeDocViewer::open(QString filepath)
{
    QFile file(filepath);
    if (file.open(QFile::ReadOnly | QFile::Text))
        mp_view->setPlainText(file.readAll());
}

void TeDocViewer::setupViewer()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    mp_view = new TeTextView();
    mp_view->setFont(font);
    mp_view->setReadOnly(true);

    mp_highlighter = new TeTextSyntaxHighlighter(mp_view->document());
}

