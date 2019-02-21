#include "TeCmdMenuSetting.h"
#include "TeViewStore.h"
#include "dialogs/TeMenuSetting.h"


TeCmdMenuSetting::TeCmdMenuSetting()
{
}


TeCmdMenuSetting::~TeCmdMenuSetting()
{
}

bool TeCmdMenuSetting::execute(TeViewStore * p_store)
{
	TeMenuSetting dlg(p_store->mainWindow());
	if (dlg.exec() == QDialog::Accepted) {

	}
	return true;
}
