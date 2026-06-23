#include "TeTextStyleEditor.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QToolButton>
#include <QColorDialog>

/**
 * @file TeTextStyleEditor.cpp
 * @brief Declaration of TeTextStyleEditor.
 * @ingroup viewer
 */


TeTextStyleEditor::TeTextStyleEditor(QWidget* parent)
	: QWidget(parent)
{
	QHBoxLayout* layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	mp_bold = new QCheckBox(tr("Bold"));
	mp_italic = new QCheckBox(tr("Italic"));
	mp_underline = new QCheckBox(tr("Underline"));
	layout->addWidget(mp_bold);
	layout->addWidget(mp_italic);
	layout->addWidget(mp_underline);

	mp_fgEnable = new QCheckBox(tr("Text"));
	mp_foreground = new QToolButton();
	mp_foreground->setToolTip(tr("Text color"));
	layout->addWidget(mp_fgEnable);
	layout->addWidget(mp_foreground);

	mp_bgEnable = new QCheckBox(tr("Back"));
	mp_background = new QToolButton();
	mp_background->setToolTip(tr("Background color"));
	layout->addWidget(mp_bgEnable);
	layout->addWidget(mp_background);

	layout->addStretch(1);

	connect(mp_bold, &QCheckBox::toggled, this, &TeTextStyleEditor::edited);
	connect(mp_italic, &QCheckBox::toggled, this, &TeTextStyleEditor::edited);
	connect(mp_underline, &QCheckBox::toggled, this, &TeTextStyleEditor::edited);

	connect(mp_fgEnable, &QCheckBox::toggled, this, [this](bool checked) {
		m_hasForeground = checked;
		updateSwatch(mp_foreground, m_foreground, m_hasForeground);
		emit edited();
	});
	connect(mp_bgEnable, &QCheckBox::toggled, this, [this](bool checked) {
		m_hasBackground = checked;
		updateSwatch(mp_background, m_background, m_hasBackground);
		emit edited();
	});

	connect(mp_foreground, &QToolButton::clicked, this, &TeTextStyleEditor::pickForeground);
	connect(mp_background, &QToolButton::clicked, this, &TeTextStyleEditor::pickBackground);

	updateSwatch(mp_foreground, m_foreground, m_hasForeground);
	updateSwatch(mp_background, m_background, m_hasBackground);
}

void TeTextStyleEditor::setFormat(const QTextCharFormat& format)
{
	mp_bold->setChecked(format.fontWeight() == QFont::Bold);
	mp_italic->setChecked(format.fontItalic());
	mp_underline->setChecked(format.fontUnderline());

	m_hasForeground = format.foreground().style() != Qt::NoBrush;
	m_foreground = m_hasForeground ? format.foreground().color() : QColor();
	m_hasBackground = format.background().style() != Qt::NoBrush;
	m_background = m_hasBackground ? format.background().color() : QColor();

	mp_fgEnable->setChecked(m_hasForeground);
	mp_bgEnable->setChecked(m_hasBackground);

	updateSwatch(mp_foreground, m_foreground, m_hasForeground);
	updateSwatch(mp_background, m_background, m_hasBackground);
}

QTextCharFormat TeTextStyleEditor::format() const
{
	QTextCharFormat format;
	if (mp_bold->isChecked()) {
		format.setFontWeight(QFont::Bold);
	}
	if (mp_italic->isChecked()) {
		format.setFontItalic(true);
	}
	if (mp_underline->isChecked()) {
		format.setFontUnderline(true);
	}
	if (m_hasForeground && m_foreground.isValid()) {
		format.setForeground(QBrush(m_foreground));
	}
	if (m_hasBackground && m_background.isValid()) {
		format.setBackground(QBrush(m_background));
	}
	return format;
}

void TeTextStyleEditor::pickForeground()
{
	QColor initial = m_foreground.isValid() ? m_foreground : Qt::black;
	QColor color = QColorDialog::getColor(initial, this, tr("Text color"));
	if (color.isValid()) {
		m_foreground = color;
		m_hasForeground = true;
		mp_fgEnable->setChecked(true);
		updateSwatch(mp_foreground, m_foreground, m_hasForeground);
		emit edited();
	}
}

void TeTextStyleEditor::pickBackground()
{
	QColor initial = m_background.isValid() ? m_background : Qt::white;
	QColor color = QColorDialog::getColor(initial, this, tr("Background color"));
	if (color.isValid()) {
		m_background = color;
		m_hasBackground = true;
		mp_bgEnable->setChecked(true);
		updateSwatch(mp_background, m_background, m_hasBackground);
		emit edited();
	}
}

void TeTextStyleEditor::updateSwatch(QToolButton* button, const QColor& color, bool enabled)
{
	if (enabled && color.isValid()) {
		button->setStyleSheet(QStringLiteral("background-color: %1;").arg(color.name()));
	} else {
		button->setStyleSheet(QString());
	}
}
