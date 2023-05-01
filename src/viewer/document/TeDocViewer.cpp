#include "TeDocViewer.h"

#include <QAction>
#include <QMenuBar>
#include <QMenu>
#include <QFile>
#include <QWebChannel>
#include <QWebEngineView>
#include <QWebChannel>

#include <QByteArray>
#include <QTextCodec>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>


#include "TeDocument.h"
#include "text/TeTextView.h"
#include "text/TeTextSyntaxHighlighter.h"
#include "text/TeTextSyntaxLoader.h"
#include "text/TeTextSyntaxDialog.h"
#include "markup/TeMarkupPage.h"
#include "markup/TeMarkupLoader.h"
#include "markup/TeMarkupContainerDialog.h"

#include <QApplication>
#include <QStyle>

#include <QDir>
#include <QDebug>


TeDocViewer::TeDocViewer(QWidget* parent)
    : QMainWindow(parent)
{
    mp_textView = nullptr;
    mp_textHighlighter = nullptr;
    mp_channel = nullptr;
    mp_page = nullptr;
    mp_webView = nullptr;

    mp_document = new TeDocument();
    mp_syntaxLoader = new TeTextSyntaxLoader();
    mp_syntaxLoader->loadAll();

    mp_markupLoader = new TeMarkupLoader();
    mp_markupLoader->loadAll();

    mp_findDialog = nullptr;
    mp_gotoLineDialog = nullptr;

    setupViewer();
    setupMenu();

    loadSettings();

    textViewMode();
    //markupMode();
}

TeDocViewer::~TeDocViewer()
{
    delete mp_document;
    delete mp_syntaxLoader;
    delete mp_channel;
    delete mp_textHighlighter;

    delete mp_textView;
    delete mp_page;
    delete mp_webView;
}

bool TeDocViewer::open(const QString& filepath)
{
    if (mp_document->load(filepath)) {
        if (centralWidget() == mp_webView) {
            markupMode();
        }
        else {
            textViewMode();
        }
        return true;
    }
    return false;
}

void TeDocViewer::reopen(const QString& codecName)
{
    mp_document->reload(codecName);
    if (centralWidget() == mp_webView) {
        markupMode();
    }
    else {
        textViewMode();
    }
}

void TeDocViewer::findNext(const QString& text, bool caseSensitive, bool regex, bool backward)
{
    if (centralWidget() == mp_textView) {
        QTextDocument::FindFlags flags;
        if (backward) flags |= QTextDocument::FindBackward;
        if (caseSensitive) flags |= QTextDocument::FindCaseSensitively;

        if (regex) {
            QRegularExpression::PatternOptions option;
            if (!caseSensitive) option |= QRegularExpression::CaseInsensitiveOption;
            QRegularExpression regexp(text, option);
            mp_textView->find(regexp, flags);
        }
        else {
            mp_textView->find(text, flags);
        }
	}
    else {
        QWebEnginePage::FindFlags flags;
        if (backward) flags |= QWebEnginePage::FindBackward;
        if (caseSensitive) flags |= QWebEnginePage::FindCaseSensitively;
        mp_webView->findText(text, flags);
	}
}

void TeDocViewer::textViewMode()
{
    if (centralWidget() == mp_webView) {
        takeCentralWidget();
        mp_webView->page()->action(QWebEnginePage::WebAction::Stop);
    }

    if (mp_document->fileinfo().exists()) {
        TeTextSyntax syntax = mp_syntaxLoader->relatedEntry(mp_document->fileinfo().suffix());
        mp_textHighlighter->setTextSyntax(syntax);
        mp_textView->setPlainText(mp_document->content());
    }
    setWindowTitle(tr("TextView"));

    if (centralWidget() != mp_textView) {
        setCentralWidget(mp_textView);
    }

    for (auto action : mp_textActions) {
		action->setEnabled(true);
	}
}

void TeDocViewer::markupMode()
{
    if (centralWidget() == mp_textView) {
        takeCentralWidget();
        mp_textView->clear();
    }

    QPoint position = pos();

    delete mp_page;
    mp_page = new TeMarkupPage();
    mp_page->setWebChannel(mp_channel);
    mp_webView->setPage(mp_page);

    if (mp_document->fileinfo().exists()) {
        QString container = mp_markupLoader->relatedContainer(mp_document->fileinfo().suffix());
        if (container.isEmpty()) {
            mp_webView->page()->setHtml(mp_document->content());
        }
        else {
            mp_webView->page()->setHtml(container);
        }
    }

    setWindowTitle(tr("MarkupView"));

    if (centralWidget() != mp_webView) {
        setCentralWidget(mp_webView);
    }

    for (auto action : mp_textActions) {
        action->setEnabled(false);
    }

    //turn back to the original position
    //this is correction for the bug of QtWebEngine at initialazing.
    move(position);
}

void TeDocViewer::showFindDialog()
{
    if (mp_findDialog == nullptr) {
        mp_findDialog = new QDialog(this);
        mp_findDialog->setWindowTitle(tr("Find"));

        QHBoxLayout* layout = new QHBoxLayout();
        layout->setContentsMargins(5, 5, 5, 5);
        layout->setSpacing(0);
        mp_findDialog->setLayout(layout);

        QLineEdit* lineEdit = new QLineEdit();
        QPushButton* btCase = new QPushButton();
        QPushButton* btRegex = new QPushButton();

        QPushButton* btBack = new QPushButton();
        btBack->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarShadeButton));
        btBack->setIconSize(QSize(10, 10));
        btBack->setFlat(true);
        connect(btBack, &QPushButton::clicked, [this, lineEdit, btCase, btRegex]() {lineEdit->setFocus();  findNext(lineEdit->text(), btCase->isChecked(), btRegex->isChecked(), true); });

        QPushButton* btForward = new QPushButton();
        btForward->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarUnshadeButton));
        btForward->setIconSize(QSize(10, 10));
        btForward->setFlat(true);
        btForward->setDefault(true);
        connect(btForward, &QPushButton::clicked, [this, lineEdit, btCase, btRegex]() {lineEdit->setFocus();  findNext(lineEdit->text(), btCase->isChecked(), btRegex->isChecked(), false); });
        QSize btSize = btForward->sizeHint();

        layout->addWidget(lineEdit);

        btCase->setText("Aa");
        // サイズを設定する
        btCase->setFlat(true);
        btCase->setCheckable(true);
        btCase->setFixedSize(btSize);
        connect(btCase, &QPushButton::clicked, [lineEdit]() {lineEdit->setFocus(); });
        layout->addWidget(btCase);

        btRegex->setText(".*");
        btRegex->setFlat(true);
        btRegex->setCheckable(true);
        btRegex->setFixedSize(btSize);
        connect(btRegex, &QPushButton::clicked, [lineEdit]() {lineEdit->setFocus(); });
        layout->addWidget(btRegex);

        layout->addWidget(btBack);
        layout->addWidget(btForward);
        lineEdit->setFocus();
        
        //clear text selection when dialog is closed.
        connect(mp_findDialog, &QDialog::finished, [this]() {
            if (centralWidget() == mp_webView) {
                mp_webView->findText(QString());
            }});
    }
    else {
        mp_findDialog->show();
    }
    
    QPoint pos = mapToGlobal(rect().topRight()-QPoint(mp_findDialog->sizeHint().width(),0));
    mp_findDialog->move(pos);
    mp_findDialog->show();
}

void TeDocViewer::showGotoLineDialog()
{
    if (mp_gotoLineDialog == nullptr) {
		mp_gotoLineDialog = new QDialog(this);
        mp_gotoLineDialog->setWindowTitle(tr("Go to Line"));
		QHBoxLayout* layout = new QHBoxLayout();
		layout->setContentsMargins(5, 5, 5, 5);
		layout->setSpacing(0);
		mp_gotoLineDialog->setLayout(layout);
		QLineEdit* lineEdit = new QLineEdit();
		QPushButton* btGo = new QPushButton();
		btGo->setText(tr("Go"));
        btGo->setContentsMargins(0, 0, 0, 0);
		btGo->setDefault(true);
		connect(btGo, &QPushButton::clicked, [this, lineEdit]() {
            //scroll to the mp_textView to the lineEdit->text().toInt()
            QTextCursor cursor = mp_textView->textCursor();
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, lineEdit->text().toInt() - 1);
            mp_textView->setTextCursor(cursor);
            mp_gotoLineDialog->close();
            });
		layout->addWidget(lineEdit);
		layout->addWidget(btGo);
		lineEdit->setFocus();
	}

    QPoint pos = mapToGlobal(rect().topRight() - QPoint(mp_gotoLineDialog->sizeHint().width(), 0));
	mp_gotoLineDialog->move(pos);
	mp_gotoLineDialog->show();
}

void TeDocViewer::closeEvent(QCloseEvent* )
{
    mp_channel->deregisterObject(mp_document);
    mp_webView->close();
}


void TeDocViewer::setupViewer()
{
    QFont font;
    font.setFamily("Courier");
    font.setFixedPitch(true);
    font.setPointSize(10);

    mp_textView = new TeTextView();
    mp_textHighlighter = new TeTextSyntaxHighlighter(mp_textView->document());
    mp_textView->setFont(font);
    mp_textView->setReadOnly(true);

    mp_webView = new QWebEngineView();
    mp_channel = new QWebChannel();

    mp_channel->registerObject("document", mp_document);
}

void TeDocViewer::setupMenu()
{
    /*
        Mode
            text   (PlainTextEdit)
            markup (WebEngineView)

        Search
            Find
            go to line
            go to top
            go to end

        Option
            Font
            Encoding   (Selector)
                UTF-8
                Shift_JIS
                EUC-JP
                ISO-2022-JP
            Highlight
            Markup Contaienr
            Word wrap   (checkbox)
            Line number (checkbox)
            Tab space   (selector)
                2
                4
                8

    */

    QMenu* menu = nullptr;
    QAction* action = nullptr;

    /////////////////////
    // Mode

    menu = menuBar()->addMenu(tr("&File"));

    action = new QAction( tr("&Text"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) {textViewMode(); });
    action->setShortcut(QKeySequence(tr("Ctrl+T")));
    menu->addAction(action);

    action = new QAction(tr("&Markup"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) {markupMode(); });
    action->setShortcut(QKeySequence(tr("Ctrl+M")));
    menu->addAction(action);

    menu->addSeparator();

    action = menu->addAction(tr("&Exit"));
    action->setShortcuts({ QKeySequence(Qt::Key_Escape) });
    connect(action, &QAction::triggered, this, &TeDocViewer::close);


    /////////////////////
    // Search
    menu = menuBar()->addMenu(tr("&Search"));
    action = new QAction(tr("&Find"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) { showFindDialog(); });
    action->setShortcut(QKeySequence(tr("Ctrl+F")));
    menu->addAction(action);

    action = new QAction(tr("&Go to line"));
    action->setEnabled(centralWidget() == mp_textView);
    mp_textActions.append(action);
    action->setShortcut(QKeySequence(tr("Ctrl+G")));
    connect(action, &QAction::triggered, [this](bool /*checked*/) { showGotoLineDialog(); });
    menu->addAction(action);

    action = new QAction(tr("&Go to top"));
    action->setEnabled(centralWidget() == mp_textView);
    mp_textActions.append(action);
    connect(action, &QAction::triggered, [this](bool /*checked*/) { mp_textView->moveCursor(QTextCursor::Start); });
    menu->addAction(action);

    action = new QAction(tr("&Go to end"));
    action->setEnabled(centralWidget() == mp_textView);
    mp_textActions.append(action);
    connect(action, &QAction::triggered, [this](bool /*checked*/) { mp_textView->moveCursor(QTextCursor::End); });
    menu->addAction(action);

    /////////////////////
    // Option
    menu = menuBar()->addMenu(tr("&Option"));

    // Font
    action = new QAction(tr("&Font"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) { fontSettings(); });
    menu->addAction(action);

    // Encoding
    QMenu* submenu = menu->addMenu(tr("&Encoding"));
    setupEncodingMenu(submenu);
    connect(mp_document, &TeDocument::codecListChanged, [this, submenu](const QStringList& /*codecList*/) { setupEncodingMenu(submenu); });
    connect(mp_document, &TeDocument::codecChanged, [this, submenu](const QString& codecName) { updateEncodingMenu(submenu, codecName); });

    // Highlight
    action = new QAction(tr("&Highlight"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) { highlightSettings(); });
    menu->addAction(action);

    // Markup container
    action = new QAction(tr("&Markup container"));
    connect(action, &QAction::triggered, [this](bool /*checked*/) { markupContainerSettings(); });
    menu->addAction(action);

    // Word wrap
    action = new QAction(tr("&Word wrap"));
    action->setCheckable(true);
    action->setChecked(mp_textView->wordWrapMode() != QTextOption::NoWrap);
    connect(action, &QAction::triggered, [this](bool checked) { mp_textView->setWordWrapMode(checked ? QTextOption::WrapAtWordBoundaryOrAnywhere : QTextOption::NoWrap); });
    menu->addAction(action);

    // Line number
    action = new QAction(tr("&Line number"));
    action->setCheckable(true);
    action->setChecked(mp_textView->isLineNumberVisible());
    connect(action, &QAction::triggered, [this](bool checked) { mp_textView->setLineNumberVisible(checked); });
    menu->addAction(action);

    // Tab space
    submenu = menu->addMenu(tr("&Tab space"));
    setupTabStopMenu(submenu);
    connect(mp_textView, &TeTextView::tabStopWidthChanged, [this, submenu](int tabStopWidth) { updateTabStopMenu(submenu, tabStopWidth); });


}

void TeDocViewer::setupEncodingMenu(QMenu* menu)
{
    menu->clear();
    QAction* action = NULL;
    for (const auto& codec : mp_document->codecList()) {
        action = new QAction(codec);
        if (mp_document->codecName() == codec) {
            action->setCheckable(true);
            action->setChecked(true);
        }
        connect(action, &QAction::triggered, [this, codec](bool /*checked*/) { reopen(codec); });
        menu->addAction(action);
    }
}

void TeDocViewer::updateEncodingMenu(QMenu* menu, const QString& codecName)
{
    for (auto action : menu->actions()) {
        if (action->text() == codecName) {
			action->setCheckable(true);
			action->setChecked(true);
		}
        else {
			action->setCheckable(false);
			action->setChecked(false);
		}
	}
}

void TeDocViewer::highlightSettings()
{
    //ToDo : highlight settings
    TeTextSyntaxDialog dialog(this);
	dialog.exec();
}

void TeDocViewer::markupContainerSettings()
{
    //ToDo : markup container settings
    TeMarkupContainerDialog dialog(this);
	dialog.exec();
}

void TeDocViewer::setupTabStopMenu(QMenu* menu)
{
    menu->clear();
	QAction* action = NULL;
    for (const auto& tabStop : { 2, 4, 8 }) {
		action = new QAction(QString::number(tabStop));
        if (mp_textView->tabStopWidth() == tabStop) {
			action->setCheckable(true);
			action->setChecked(true);
		}
		connect(action, &QAction::triggered, [this, tabStop](bool /*checked*/) { mp_textView->setTabStopWidth(tabStop); });
		menu->addAction(action);
	}
}

void TeDocViewer::updateTabStopMenu(QMenu* menu, int tabStop)
{
    for (auto action : menu->actions()) {
        if (action->text() == QString::number(tabStop)) {
			action->setCheckable(true);
			action->setChecked(true);
		}
        else {
			action->setCheckable(false);
			action->setChecked(false);
		}
	}
}

void TeDocViewer::loadSettings()
{
    //ToDo : load settings
}

void TeDocViewer::fontSettings()
{
    bool ok;
	QFont font = QFontDialog::getFont(&ok, mp_textView->font(), this);
    if (ok) {
		mp_textView->setFont(font);
	}
}

