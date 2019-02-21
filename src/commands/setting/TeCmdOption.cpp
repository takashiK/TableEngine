#include "TeCmdOption.h"
#include "TeViewStore.h"
#include "dialogs/TeOptionSetting.h"
#include <QDialog>

TeCmdOption::TeCmdOption()
{
}


TeCmdOption::~TeCmdOption()
{
}

bool TeCmdOption::execute(TeViewStore * p_store)
{
	TeOptionSetting dlg(p_store->mainWindow());
	if (dlg.exec() == QDialog::Accepted) {
		//設定を反映
		p_store->loadSetting();
	}
	return true;
}
