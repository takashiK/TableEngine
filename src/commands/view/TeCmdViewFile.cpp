#include "TeCmdViewFile.h"

#include "viewer/document/TeDocViewer.h"
#include "viewer/picture/TePictureViewer.h"
#include "viewer/binary/TeBinaryViewer.h"
#include "TeUtils.h"
#include "TeViewStore.h"

#include <QMessageBox>


TeCmdViewFile::TeCmdViewFile()
{
}

TeCmdViewFile::~TeCmdViewFile()
{
}

bool TeCmdViewFile::isAvailable()
{
	// this command need selected item.
	return false;
}

bool TeCmdViewFile::execute(TeViewStore* p_store)
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
