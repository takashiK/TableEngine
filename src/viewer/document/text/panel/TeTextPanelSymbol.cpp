#include "TeTextPanelSymbol.h"

#include "TeTextStyleEditor.h"

#include <QFormLayout>
#include <QLineEdit>

/**
 * @file TeTextPanelSymbol.cpp
 * @brief Declaration of TeTextPanelSymbol.
 * @ingroup viewer
 */


TeTextPanelSymbol::TeTextPanelSymbol(QWidget* parent)
	: TeTextPanelItem(tr("Symbol"), parent)
{
	mp_keywords = new QLineEdit();
	mp_keywords->setPlaceholderText(tr("comma separated keywords"));
	contentLayout()->addRow(tr("Keywords:"), mp_keywords);

	mp_style = new TeTextStyleEditor();
	contentLayout()->addRow(tr("Style:"), mp_style);

	connect(mp_keywords, &QLineEdit::textEdited, this, &TeTextPanelSymbol::edited);
	connect(mp_style, &TeTextStyleEditor::edited, this, &TeTextPanelSymbol::edited);
}

void TeTextPanelSymbol::setData(const TeTextSyntax::SyntaxKeywords& data)
{
	mp_keywords->setText(data.keywords.join(QStringLiteral(", ")));
	mp_style->setFormat(data.format);
}

TeTextSyntax::SyntaxKeywords TeTextPanelSymbol::data() const
{
	TeTextSyntax::SyntaxKeywords result;
	result.format = mp_style->format();

	const QStringList parts = mp_keywords->text().split(QLatin1Char(','), Qt::SkipEmptyParts);
	for (const QString& part : parts) {
		const QString trimmed = part.trimmed();
		if (!trimmed.isEmpty()) {
			result.keywords.append(trimmed);
		}
	}
	return result;
}
