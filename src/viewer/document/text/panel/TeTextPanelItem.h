#pragma once

#include <QFrame>

class QFormLayout;

/**
 * @file TeTextPanelItem.h
 * @brief Declaration of TeTextPanelItem.
 * @ingroup viewer
 */

/**
 * @class TeTextPanelItem
 * @brief Base class for a single syntax entry widget shown inside a
 *        TeTextPanelList.
 * @ingroup viewer
 *
 * @details Provides the common header row (title plus move-up/move-down/remove
 * buttons) and a form-style content area. Subclasses populate the content area
 * via contentLayout() and map their editing widgets to/from the underlying
 * TeTextSyntax data structures.
 */
class TeTextPanelItem : public QFrame
{
	Q_OBJECT

public:
	explicit TeTextPanelItem(const QString& title, QWidget* parent = nullptr);
	~TeTextPanelItem() override;

signals:
	void moveUpRequested();
	void moveDownRequested();
	void removeRequested();
	void edited();

protected:
	QFormLayout* contentLayout() const;

private:
	QFormLayout* mp_contentLayout = nullptr;
};
