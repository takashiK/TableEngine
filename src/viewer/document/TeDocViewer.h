#pragma once

#include <QMainWindow>

class QWebEngineView;
class QWebChannel;

class TeTextView;
class TeMarkupPage;
class TeMarkupLoader;
class TeTextSyntaxHighlighter;
class TeTextSyntaxLoader;
class TeDocument;


class TeDocViewer :
    public QMainWindow
{
    Q_OBJECT

public:
    TeDocViewer(QWidget* parent = 0);
    virtual ~TeDocViewer();

    bool open(const QString& filepath);
    void reopen(const QString& codecName);

public slots:
    void findNext(const QString& text, bool caseSensitive, bool regex, bool backward=false);

protected:
    void textViewMode();
    void markupMode();
    void showFindDialog();
    void showGotoLineDialog();

    virtual void	closeEvent(QCloseEvent* event);

private:
    void setupViewer();
    void setupMenu();
    void setupEncodingMenu(QMenu* menu);
    void updateEncodingMenu(QMenu* menu, const QString& codecName);
    void highlightSettings();
    void markupContainerSettings();
    void setupTabStopMenu(QMenu* menu);
    void updateTabStopMenu(QMenu* menu, int tabStop);

    void loadSettings();
    void fontSettings();

    TeTextView* mp_textView;
    QWebChannel* mp_channel;
    TeMarkupPage* mp_page;
    QWebEngineView* mp_webView;
    TeDocument* mp_document;
    TeTextSyntaxHighlighter* mp_textHighlighter;
    TeTextSyntaxLoader* mp_syntaxLoader;
    TeMarkupLoader* mp_markupLoader;
    QList<QAction*> mp_textActions;
    QDialog* mp_findDialog;
    QDialog* mp_gotoLineDialog;
};

