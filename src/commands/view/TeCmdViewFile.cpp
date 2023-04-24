#include "TeCmdViewFile.h"

#include "TeDocViewer.h"
#include "TeUtils.h"
#include "TeViewStore.h"

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
			TeDocViewer* docViewer = new TeDocViewer();
			if (docViewer->open(path)) {
				p_store->registerFloatingWidget(docViewer);
				docViewer->show();
			}
			else {
				delete docViewer;
			}
		}
	}

	//finished
	return true;
}
