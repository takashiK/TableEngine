#include "TeTextPanelItem.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QToolButton>

/**
 * @file TeTextPanelItem.cpp
 * @brief Declaration of TeTextPanelItem.
 * @ingroup viewer
 */


TeTextPanelItem::TeTextPanelItem(const QString& title, QWidget* parent)
	: QFrame(parent)
{
	setFrameShape(QFrame::StyledPanel);

	QVBoxLayout* mainLayout = new QVBoxLayout(this);

	// Header row: title and reorder/remove controls.
	QHBoxLayout* headerLayout = new QHBoxLayout();
	QLabel* titleLabel = new QLabel(title);
	headerLayout->addWidget(titleLabel);
	headerLayout->addStretch(1);

	QToolButton* upBtn = new QToolButton();
	upBtn->setArrowType(Qt::UpArrow);
	upBtn->setToolTip(tr("Move up"));
	headerLayout->addWidget(upBtn);

	QToolButton* downBtn = new QToolButton();
	downBtn->setArrowType(Qt::DownArrow);
	downBtn->setToolTip(tr("Move down"));
	headerLayout->addWidget(downBtn);

	QToolButton* delBtn = new QToolButton();
	delBtn->setText(QStringLiteral("\u00D7"));
	delBtn->setToolTip(tr("Remove"));
	headerLayout->addWidget(delBtn);

	mainLayout->addLayout(headerLayout);

	// Content area filled by subclasses.
	mp_contentLayout = new QFormLayout();
	mainLayout->addLayout(mp_contentLayout);

	connect(upBtn, &QToolButton::clicked, this, &TeTextPanelItem::moveUpRequested);
	connect(downBtn, &QToolButton::clicked, this, &TeTextPanelItem::moveDownRequested);
	connect(delBtn, &QToolButton::clicked, this, &TeTextPanelItem::removeRequested);
}

TeTextPanelItem::~TeTextPanelItem() = default;

QFormLayout* TeTextPanelItem::contentLayout() const
{
	return mp_contentLayout;
}
