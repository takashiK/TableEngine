#include "TeWindowsEventFilter.h"
#include "platform/TeNativeEvent.h"
#include "TeUtils.h"

#include <QApplication>

#include <Windows.h>
#include <winuser.h>
#include <dbt.h>

TeWindowsEventFilter::TeWindowsEventFilter()
{
	mp_nativeEvent = nullptr;
}

TeWindowsEventFilter::~TeWindowsEventFilter()
{}

bool TeWindowsEventFilter::nativeEventFilter(const QByteArray & eventType, void* message, qintptr * result)
{
	NOT_USED(result);

	if (mp_nativeEvent != nullptr && eventType == "windows_generic_MSG")
	{
		MSG* msg = reinterpret_cast<MSG*>(message);
		if (msg->message == WM_DEVICECHANGE)
		{
			switch (msg->wParam)
			{
				case DBT_DEVICEARRIVAL:
					mp_nativeEvent->changeMountState(true);
					break;
				case DBT_DEVICEREMOVECOMPLETE:
					mp_nativeEvent->changeMountState(false);
					break;
			}
		}
	}
	return false;
}

void TeWindowsEventFilter::setNativeEvent(TeNativeEvent* p_event)
{
	mp_nativeEvent = p_event;
}
