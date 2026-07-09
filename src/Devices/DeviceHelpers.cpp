#include "DeviceHelpers.h"

int setDualKeyImage(StreamDock &device, int key, const uint8_t *data, size_t length) {
    if (key < 1 || key > device.image_keys()) {
        return -1;
    }
    const int hardware_key = device.get_image_key(static_cast<ButtonKey>(key));
    if (hardware_key < 0) {
        return -1;
    }
    device.transport.setKeyImageStream(data, length, hardware_key);
    return 0;
}

void configureReport1025(LibUSBHIDAPI &transport) {
    transport.setReportSize(513, 1025, 0);
    transport.setReportId(0x00);
}

void configureReport513(LibUSBHIDAPI &transport) {
    transport.setReportSize(513, 513, 0);
    transport.setReportId(0x00);
}
