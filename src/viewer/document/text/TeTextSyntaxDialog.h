#pragma once

#include <QDialog>

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

private:
	void rebuildForTitle(const QString& title);

	TeTextSyntaxHighlighter* mp_textHighlighter = nullptr;

	TeTextSyntaxLoader m_loader;

	QComboBox* mp_ruleCombo = nullptr;
	QListWidget* mp_suffixList = nullptr;
	TeTextPanelList* mp_symbolList = nullptr;
	TeTextPanelList* mp_syntaxList = nullptr;
	TeTextPanelList* mp_regionList = nullptr;
};

