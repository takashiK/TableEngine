#pragma once

#include <QDialog>

class TeTextSyntaxHighlighter;

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
	TeTextSyntaxHighlighter* mp_textHighlighter = nullptr;
};

