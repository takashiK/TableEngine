#pragma once

#include <QWidget>
#include <QList>

class TeTextPanelItem;
class QVBoxLayout;
class QPushButton;

/**
 * @file TeTextPanelList.h
 * @brief Declaration of TeTextPanelList.
 * @ingroup viewer
 */

/**
 * @class TeTextPanelList
 * @brief Ordered container of TeTextPanelItem widgets with a titled header and
 *        an add button.
 * @ingroup viewer
 *
 * @details Owns the layout of its child entry widgets and handles in-group
 * reordering and removal triggered by each item. The container is agnostic to
 * the concrete entry type: pressing the add button emits itemAdded() so the
 * owner can create and append the matching panel via addPanel().
 */
class TeTextPanelList : public QWidget
{
	Q_OBJECT

public:
	explicit TeTextPanelList(const QString& title, QWidget* parent = nullptr);

	void addPanel(TeTextPanelItem* item);
	void clearPanels();

	int count() const;
	TeTextPanelItem* panelAt(int index) const;
	int indexOf(TeTextPanelItem* item) const;

signals:
	void itemAdded();
	void itemMoved(int from, int to);
	void itemRemoved(int index);
	void edited();

private:
	void moveItem(TeTextPanelItem* item, int delta);
	void removeItem(TeTextPanelItem* item);

	QVBoxLayout* mp_itemLayout = nullptr;
	QPushButton* mp_addButton = nullptr;
	QList<TeTextPanelItem*> m_items;
};
