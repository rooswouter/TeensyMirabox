#include "StreamDock293.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDock293::set_device() {
    configureReport513(transport);
    feature_option.deviceType = device_type::dock_293;
}

int StreamDock293::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDock293::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDock293::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundBitmap(data, length);
    return 0;
}

int StreamDock293::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_15, DeviceKeyMaps::REMAP_15_COUNT, logical_key);
}

InputEvent StreamDock293::decode_input_event(int hardware_code, int state) {
    return DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_15, DeviceKeyMaps::REMAP_15_COUNT, hardware_code, state);
}

ImageFormat StreamDock293::key_image_format() const {
    return {100, 100, ImageFileFormat::JPEG, 180, false, false};
}

ImageFormat StreamDock293::touchscreen_image_format() const {
    return {800, 480, ImageFileFormat::JPEG, 180, false, false};
}
