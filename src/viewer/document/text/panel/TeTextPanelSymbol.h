#pragma once

#include "TeTextPanelItem.h"

#include "viewer/document/text/TeTextSyntax.h"

class QLineEdit;
class TeTextStyleEditor;

/**
 * @file TeTextPanelSymbol.h
 * @brief Declaration of TeTextPanelSymbol.
 * @ingroup viewer
 */

/**
 * @class TeTextPanelSymbol
 * @brief Entry widget editing a TeTextSyntax::SyntaxKeywords definition.
 * @ingroup viewer
 */
class TeTextPanelSymbol : public TeTextPanelItem
{
	Q_OBJECT

public:
	explicit TeTextPanelSymbol(QWidget* parent = nullptr);

	void setData(const TeTextSyntax::SyntaxKeywords& data);
	TeTextSyntax::SyntaxKeywords data() const;

private:
	QLineEdit* mp_keywords = nullptr;
	TeTextStyleEditor* mp_style = nullptr;
};
