#include "TeCmdViewFile.h"

#include "viewer/document/TeDocViewer.h"
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
			else {
				// show message dialog box.
				QMessageBox::warning(p_store->mainWindow(), "Unsupported file", path);
			}
		}
	}

	//finished
	return true;
}
