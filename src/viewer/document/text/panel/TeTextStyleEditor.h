#pragma once

#include <QWidget>
#include <QTextCharFormat>
#include <QColor>

class QCheckBox;
class QToolButton;

/**
 * @file TeTextStyleEditor.h
 * @brief Declaration of TeTextStyleEditor.
 * @ingroup viewer
 */

/**
 * @class TeTextStyleEditor
 * @brief Compact editor for a QTextCharFormat shared by the syntax panels.
 * @ingroup viewer
 *
 * @details Exposes bold/italic/underline toggles plus optional foreground and
 * background colors. Only the enabled attributes are written into the format
 * returned by format(), matching the persisted highlight definition semantics.
 */
class TeTextStyleEditor : public QWidget
{
	Q_OBJECT

public:
	explicit TeTextStyleEditor(QWidget* parent = nullptr);

	void setFormat(const QTextCharFormat& format);
	QTextCharFormat format() const;

signals:
	void edited();

private:
	void pickForeground();
	void pickBackground();
	void updateSwatch(QToolButton* button, const QColor& color, bool enabled);

	QCheckBox* mp_bold = nullptr;
	QCheckBox* mp_italic = nullptr;
	QCheckBox* mp_underline = nullptr;
	QCheckBox* mp_fgEnable = nullptr;
	QCheckBox* mp_bgEnable = nullptr;
	QToolButton* mp_foreground = nullptr;
	QToolButton* mp_background = nullptr;

	QColor m_foreground;
	QColor m_background;
	bool m_hasForeground = false;
	bool m_hasBackground = false;
};
