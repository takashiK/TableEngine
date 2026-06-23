#include "TeTextPanelList.h"

#include "TeTextPanelItem.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

/**
 * @file TeTextPanelList.cpp
 * @brief Declaration of TeTextPanelList.
 * @ingroup viewer
 */


TeTextPanelList::TeTextPanelList(const QString& title, QWidget* parent)
	: QWidget(parent)
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	// Header row: group title and add button.
	QHBoxLayout* headerLayout = new QHBoxLayout();
	QLabel* titleLabel = new QLabel(title);
	QFont titleFont = titleLabel->font();
	titleFont.setBold(true);
	titleLabel->setFont(titleFont);
	headerLayout->addWidget(titleLabel);
	headerLayout->addStretch(1);

	mp_addButton = new QPushButton(tr("Add"));
	headerLayout->addWidget(mp_addButton);

	mainLayout->addLayout(headerLayout);

	mp_itemLayout = new QVBoxLayout();
	mainLayout->addLayout(mp_itemLayout);

	connect(mp_addButton, &QPushButton::clicked, this, &TeTextPanelList::itemAdded);
}

void TeTextPanelList::addPanel(TeTextPanelItem* item)
{
	if (!item) {
		return;
	}

	m_items.append(item);
	mp_itemLayout->addWidget(item);

	connect(item, &TeTextPanelItem::moveUpRequested, this, [this, item]() {
		moveItem(item, -1);
	});
	connect(item, &TeTextPanelItem::moveDownRequested, this, [this, item]() {
		moveItem(item, 1);
	});
	connect(item, &TeTextPanelItem::removeRequested, this, [this, item]() {
		removeItem(item);
	});
	connect(item, &TeTextPanelItem::edited, this, &TeTextPanelList::edited);
}

void TeTextPanelList::clearPanels()
{
	for (TeTextPanelItem* item : m_items) {
		mp_itemLayout->removeWidget(item);
		item->deleteLater();
	}
	m_items.clear();
}

int TeTextPanelList::count() const
{
	return m_items.size();
}

TeTextPanelItem* TeTextPanelList::panelAt(int index) const
{
	if (index >= 0 && index < m_items.size()) {
		return m_items.at(index);
	}
	return nullptr;
}

int TeTextPanelList::indexOf(TeTextPanelItem* item) const
{
	return m_items.indexOf(item);
}

void TeTextPanelList::moveItem(TeTextPanelItem* item, int delta)
{
	const int from = m_items.indexOf(item);
	if (from < 0) {
		return;
	}

	const int to = from + delta;
	if (to < 0 || to >= m_items.size()) {
		return;
	}

	m_items.move(from, to);
	mp_itemLayout->removeWidget(item);
	mp_itemLayout->insertWidget(to, item);

	emit itemMoved(from, to);
}

void TeTextPanelList::removeItem(TeTextPanelItem* item)
{
	const int index = m_items.indexOf(item);
	if (index < 0) {
		return;
	}

	m_items.removeAt(index);
	mp_itemLayout->removeWidget(item);
	item->deleteLater();

	emit itemRemoved(index);
}
