#pragma once

#include "TeTextPanelItem.h"

#include "viewer/document/text/TeTextSyntax.h"

class QLineEdit;
class TeTextStyleEditor;

/**
 * @file TeTextPanelRegion.h
 * @brief Declaration of TeTextPanelRegion.
 * @ingroup viewer
 */

/**
 * @class TeTextPanelRegion
 * @brief Entry widget editing a TeTextSyntax::SyntaxRegion definition.
 * @ingroup viewer
 */
class TeTextPanelRegion : public TeTextPanelItem
{
	Q_OBJECT

public:
	explicit TeTextPanelRegion(QWidget* parent = nullptr);

	void setData(const TeTextSyntax::SyntaxRegion& data);
	TeTextSyntax::SyntaxRegion data() const;

private:
	QLineEdit* mp_startRegex = nullptr;
	QLineEdit* mp_endRegex = nullptr;
	TeTextStyleEditor* mp_style = nullptr;
};
