#include "TeCmdToolBinary.h"

#include "viewer/document/TeDocViewer.h"
#include "viewer/picture/TePictureViewer.h"
#include "viewer/binary/TeBinaryViewer.h"
#include "utils/TeUtils.h"
#include "TeViewStore.h"

#include <QMessageBox>

/**
 * @file TeCmdToolBinary.cpp
 * @brief Implementation of TeCmdToolBinary.
 * @ingroup commands
 */


TeCmdToolBinary::TeCmdToolBinary()
{
}

TeCmdToolBinary::~TeCmdToolBinary()
{
}

bool TeCmdToolBinary::isActive( TeViewStore* p_store )
{
	NOT_USED(p_store);
	return false;
}

bool TeCmdToolBinary::isSelected(TeViewStore* p_store, const TeCmdParam* p_cmdParam)
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdToolBinary::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_SELECT

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_OTHER

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_DIRECTORY
	);
}

bool TeCmdToolBinary::execute(TeViewStore* p_store)
{
	// get the selected item
	QStringList paths;

	if (getSelectedItemList(p_store, &paths)) {
		for (auto& path : paths) {
			TeBinaryViewer* binaryViewer = new TeBinaryViewer();
			if (binaryViewer->open(path)) {
				p_store->registerFloatingWidget(binaryViewer);
				binaryViewer->show();
			}
			else {
				delete binaryViewer;
			}
		}
	}

	//finished
	return true;
}
