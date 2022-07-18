#include "TeDocViewer.h"

#include <QAction>
#include <QMenuBar>
#include <QMenu>

#include "text/TeTextView.h"
#include "text/TeTextSyntaxHighlighter.h"

TeDocViewer::TeDocViewer(QWidget* parent)
    : QMainWindow(parent)
{
    mp_textView = new TeTextView();
    mp_textHighlighter = new TeTextSyntaxHighlighter(mp_textView->document());

    setupViewer();
    setupMenu();

    setCentralWidget(mp_textView);
    setWindowTitle(tr("TextView"));
}

void TeDocViewer::open(QString filepath)
{
    QFile file(filepath);
    if (file.open(QFile::ReadOnly | QFile::Text))
        mp_textView->setPlainText(file.readAll());
}

void TeDocViewer::textViewMode()
{
}

void TeDocViewer::richViewMode()
{
}

void TeDocViewer::setupViewer()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    mp_textView->setFont(font);
    mp_textView->setReadOnly(true);
}

void TeDocViewer::setupMenu()
{
    /*
        Mode
            Plain Text
            Rich Text

        Encode
            ASCII
            UTF-8
            UTF-16
            SJIS
            EUC-JP

        View
            Find
            go to
            go top
            go end

        Option
            Font
            Highlight
            Word wrap
            Line number
            Tab space
                2
                4
                8

    */

    QMenu* menu = NULL;
    QAction* action = NULL;

    menu = menuBar()->addMenu("&Settings");

    action = new QAction( tr("&Text"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) {textViewMode(); });
    menu->addAction(action);

    action = new QAction(tr("&Html"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) {richViewMode(); });
    menu->addAction(action);
}

