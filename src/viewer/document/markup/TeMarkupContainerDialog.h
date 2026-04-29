#pragma once

#include <QDialog>

/**
 * @file TeMarkupContainerDialog.h
 * @brief Declaration of TeMarkupContainerDialog.
 * @ingroup viewer
 */


class TeMarkupContainerDialog  : public QDialog
{
	Q_OBJECT

public:
	TeMarkupContainerDialog(QWidget *parent);
	virtual ~TeMarkupContainerDialog();
};
