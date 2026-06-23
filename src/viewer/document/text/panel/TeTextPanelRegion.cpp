#include "TeTextPanelRegion.h"

#include "TeTextStyleEditor.h"

#include <QFormLayout>
#include <QLineEdit>

/**
 * @file TeTextPanelRegion.cpp
 * @brief Declaration of TeTextPanelRegion.
 * @ingroup viewer
 */


TeTextPanelRegion::TeTextPanelRegion(QWidget* parent)
	: TeTextPanelItem(tr("Region"), parent)
{
	mp_startRegex = new QLineEdit();
	mp_startRegex->setPlaceholderText(tr("start regular expression"));
	contentLayout()->addRow(tr("Start:"), mp_startRegex);

	mp_endRegex = new QLineEdit();
	mp_endRegex->setPlaceholderText(tr("end regular expression"));
	contentLayout()->addRow(tr("End:"), mp_endRegex);

	mp_style = new TeTextStyleEditor();
	contentLayout()->addRow(tr("Style:"), mp_style);

	connect(mp_startRegex, &QLineEdit::textEdited, this, &TeTextPanelRegion::edited);
	connect(mp_endRegex, &QLineEdit::textEdited, this, &TeTextPanelRegion::edited);
	connect(mp_style, &TeTextStyleEditor::edited, this, &TeTextPanelRegion::edited);
}

void TeTextPanelRegion::setData(const TeTextSyntax::SyntaxRegion& data)
{
	mp_startRegex->setText(data.startRegex.pattern());
	mp_endRegex->setText(data.endRegex.pattern());
	mp_style->setFormat(data.format);
}

TeTextSyntax::SyntaxRegion TeTextPanelRegion::data() const
{
	TeTextSyntax::SyntaxRegion result;
	result.format = mp_style->format();
	result.startRegex = QRegularExpression(mp_startRegex->text());
	result.endRegex = QRegularExpression(mp_endRegex->text());
	return result;
}
