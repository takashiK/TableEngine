#include "TeBinaryViewer.h"

#include <qhexview.h>
#include <document/buffer/qmemorybuffer.h>

TeBinaryViewer::TeBinaryViewer(QWidget *parent)
	: QMainWindow(parent)
{
	mp_hexDocument = nullptr;
	mp_hexEdit = new QHexView();
	mp_hexEdit->setReadOnly(true);

	setCentralWidget(mp_hexEdit);
}

TeBinaryViewer::~TeBinaryViewer()
{
	delete mp_hexDocument;
}

bool TeBinaryViewer::open(const QString& path)
{
	mp_hexDocument = QHexDocument::fromFile<QMemoryBuffer>(path);
	if (mp_hexDocument)
	{
		mp_hexEdit->setDocument(mp_hexDocument);
		return true;
	}
	return false;
}
