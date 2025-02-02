#include "TeNativeEvent.h"

TeNativeEvent::TeNativeEvent(QObject *parent)
	: QObject(parent)
{}

TeNativeEvent::~TeNativeEvent()
{}

void TeNativeEvent::changeMountState(bool state)
{
	emit mountStateChanged(state);
}
