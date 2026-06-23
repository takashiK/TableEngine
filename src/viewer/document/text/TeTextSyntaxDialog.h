#pragma once

#include <QDialog>

#include "TeTextSyntax.h"
#include "TeTextSyntaxLoader.h"

class TeTextSyntaxHighlighter;
class TeTextPanelList;
class QComboBox;
class QListWidget;

/**
 * @file TeTextSyntaxDialog.h
 * @brief Declaration of TeTextSyntaxDialog.
 * @ingroup viewer
 */

class TeTextSyntaxDialog :
    public QDialog
{
	Q_OBJECT

public:
	TeTextSyntaxDialog(QWidget *parent);
	virtual ~TeTextSyntaxDialog();

	void setSyntaxHighlighter(TeTextSyntaxHighlighter* highlighter);

protected:
	void done(int result) override;

private:
	void rebuildForTitle(const QString& title);
	void reloadSuffixList(const QString& title);

	TeTextSyntax collectCurrentSyntax() const;
	void commitCurrent();
	void onAddEntry();
	void onDelEntry();
	void onAddSuffix();
	void onDelSuffix();

	TeTextSyntaxHighlighter* mp_textHighlighter = nullptr;

	TeTextSyntaxLoader m_loader;

	QString m_currentTitle;
	bool m_suppressComboSignal = false;

	QComboBox* mp_ruleCombo = nullptr;
	QListWidget* mp_suffixList = nullptr;
	TeTextPanelList* mp_symbolList = nullptr;
	TeTextPanelList* mp_syntaxList = nullptr;
	TeTextPanelList* mp_regionList = nullptr;
};

