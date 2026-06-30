#pragma once

#include "TeTextPanelItem.h"

#include "viewer/document/text/TeTextSyntax.h"

class QLineEdit;
class TeTextStyleEditor;

/**
 * @file TeTextPanelSyntax.h
 * @brief Declaration of TeTextPanelSyntax.
 * @ingroup viewer
 */

/**
 * @class TeTextPanelSyntax
 * @brief Entry widget editing a TeTextSyntax::SyntaxRegex definition.
 * @ingroup viewer
 */
class TeTextPanelSyntax : public TeTextPanelItem
{
	Q_OBJECT

public:
	explicit TeTextPanelSyntax(QWidget* parent = nullptr);

	void setData(const TeTextSyntax::SyntaxRegex& data);
	TeTextSyntax::SyntaxRegex data() const;

private:
	QLineEdit* mp_regex = nullptr;
	TeTextStyleEditor* mp_style = nullptr;
};
