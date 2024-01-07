#include "TeCmdToolBinary.h"

#include "viewer/document/TeDocViewer.h"
#include "viewer/picture/TePictureViewer.h"
#include "viewer/binary/TeBinaryViewer.h"
#include "TeUtils.h"
#include "TeViewStore.h"

#include <QMessageBox>


TeCmdToolBinary::TeCmdToolBinary()
{
}

TeCmdToolBinary::~TeCmdToolBinary()
{
}

bool TeCmdToolBinary::isActive( TeViewStore* p_store )
{
	return false;
}

QFlags<TeTypes::CmdType> TeCmdToolBinary::type()
{
	return QFlags<TeTypes::CmdType>(
		TeTypes::CMD_TRIGGER_NORMAL
		// TeTypes::CMD_TRIGGER_TOGGLE
		// TeTypes::CMD_TRIGGER_SELECTION

		| TeTypes::CMD_CATEGORY_TREE
		| TeTypes::CMD_CATEGORY_LIST
		| TeTypes::CMD_CATEGORY_NONE

		| TeTypes::CMD_TARGET_FILE
		| TeTypes::CMD_TARGET_ARCHIVE
	);
}

QList<TeMenuParam> TeCmdToolBinary::menuParam()
{
	return QList<TeMenuParam>();
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
