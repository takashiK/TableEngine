#pragma once

#include <QDialog>

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
};

