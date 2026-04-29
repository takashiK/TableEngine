#include "TeNativeEvent.h"

/**
 * @file TeNativeEvent.cpp
 * @brief Declaration of TeNativeEvent.
 * @ingroup platform
 */


TeNativeEvent::TeNativeEvent(QObject *parent)
	: QObject(parent)
{}

TeNativeEvent::~TeNativeEvent()
{}

void TeNativeEvent::changeMountState(bool state)
{
	emit mountStateChanged(state);
}
