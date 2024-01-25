#include "TeCmdToolFile.h"

#include "viewer/document/TeDocViewer.h"
#include "viewer/picture/TePictureViewer.h"
#include "viewer/binary/TeBinaryViewer.h"
#include "TeUtils.h"
#include "TeViewStore.h"

#include <QMessageBox>


TeCmdToolFile::TeCmdToolFile()
{
}

TeCmdToolFile::~TeCmdToolFile()
{
}

bool TeCmdToolFile::isActive( TeViewStore* p_store )
{
	NOT_USED(p_store);
	return false;
}

QFlags<TeTypes::CmdType> TeCmdToolFile::type()
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

QList<TeMenuParam> TeCmdToolFile::menuParam()
{
	return QList<TeMenuParam>();
}

bool TeCmdToolFile::execute(TeViewStore* p_store)
{
	// get the selected item
	QStringList paths;

	if (getSelectedItemList(p_store, &paths)) {
		for (auto& path : paths) {
			TeFileType type = getFileType(path);
			if (type == TE_FILE_TEXT){
					TeDocViewer* docViewer = new TeDocViewer();
				if (docViewer->open(path)) {
					p_store->registerFloatingWidget(docViewer);
					docViewer->show();
				}
				else {
					delete docViewer;
				}
			}
			else if (type == TE_FILE_IMAGE) {
				TePictureViewer* pictureViewer = new TePictureViewer();
				if (pictureViewer->open(path)) {
					p_store->registerFloatingWidget(pictureViewer);
					pictureViewer->show();
				}
				else {
					delete pictureViewer;
				}
			}
			else if (type == TE_FILE_ARCHIVE) {
				p_store->createArchiveFolderView(path);
			}
			else {
				// show message dialog box.
				int result = QMessageBox::warning(p_store->mainWindow(), "Unsupported file.\n open in hex viewer?", path, QMessageBox::Ok,QMessageBox::Cancel);
				if (result == QMessageBox::Ok) {
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
		}
	}

	//finished
	return true;
}
