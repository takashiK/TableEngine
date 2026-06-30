#pragma once

#include <QMainWindow>

/**
 * @file TeDocViewer.h
 * @brief Declaration of TeDocViewer.
 * @ingroup viewer
 */


class TeTextView;
class TeTextSyntaxHighlighter;
class TeTextSyntaxLoader;
class TeDocument;

class QTextEdit;
class QCloseEvent;

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
    void closeEvent(QCloseEvent* event) override;
    void textViewMode();
    void markupMode();
    void showFindDialog();
    void showGotoLineDialog();

private:
    void setupViewer();
    void setupMenu();
    void setupEncodingMenu(QMenu* menu);
    void updateEncodingMenu(QMenu* menu, const QString& codecName);
    void highlightSettings();
    void setupTabStopMenu(QMenu* menu);
    void updateTabStopMenu(QMenu* menu, int tabStop);

    void loadSettings();
    void fontSettings();

    TeTextView* mp_textView = nullptr;
    QTextEdit* mp_markupView = nullptr;
    TeDocument* mp_document;
    TeTextSyntaxHighlighter* mp_textHighlighter = nullptr;
    TeTextSyntaxLoader* mp_syntaxLoader;
    QList<QAction*> mp_textActions;
    QDialog* mp_findDialog = nullptr;
    QDialog* mp_gotoLineDialog = nullptr;
    QString m_textCodec;
};

