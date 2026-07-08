#pragma once

#include "StreamDock.h"

int setDualKeyImage(StreamDock &device, int key, const uint8_t *data, size_t length);
void configureReport1025(LibUSBHIDAPI &transport);
void configureReport513(LibUSBHIDAPI &transport);
