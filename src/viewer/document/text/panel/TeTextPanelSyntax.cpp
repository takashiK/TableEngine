#include "TeTextPanelSyntax.h"

#include "TeTextStyleEditor.h"

#include <QFormLayout>
#include <QLineEdit>

/**
 * @file TeTextPanelSyntax.cpp
 * @brief Declaration of TeTextPanelSyntax.
 * @ingroup viewer
 */


TeTextPanelSyntax::TeTextPanelSyntax(QWidget* parent)
	: TeTextPanelItem(tr("Syntax"), parent)
{
	mp_regex = new QLineEdit();
	mp_regex->setPlaceholderText(tr("regular expression"));
	contentLayout()->addRow(tr("Regex:"), mp_regex);

	mp_style = new TeTextStyleEditor();
	contentLayout()->addRow(tr("Style:"), mp_style);

	connect(mp_regex, &QLineEdit::textEdited, this, &TeTextPanelSyntax::edited);
	connect(mp_style, &TeTextStyleEditor::edited, this, &TeTextPanelSyntax::edited);
}

void TeTextPanelSyntax::setData(const TeTextSyntax::SyntaxRegex& data)
{
	mp_regex->setText(data.regex.pattern());
	mp_style->setFormat(data.format);
}

TeTextSyntax::SyntaxRegex TeTextPanelSyntax::data() const
{
	TeTextSyntax::SyntaxRegex result;
	result.format = mp_style->format();
	result.regex = QRegularExpression(mp_regex->text());
	return result;
}
