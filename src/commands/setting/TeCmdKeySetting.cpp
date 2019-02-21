#include "TeCmdKeySetting.h"
#include "TeViewStore.h"
#include "dialogs/TeKeySetting.h"


TeCmdKeySetting::TeCmdKeySetting()
{
}


TeCmdKeySetting::~TeCmdKeySetting()
{
}

bool TeCmdKeySetting::execute(TeViewStore * p_store)
{
	TeKeySetting dlg(p_store->mainWindow());
	if (dlg.exec() == QDialog::Accepted) {
		p_store->loadKeySetting();
	}
	return true;
}
