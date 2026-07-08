#include "StreamDock293.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDock293V3::set_device() {
    configureReport1025(transport);
    feature_option.deviceType = device_type::dock_293v3;
}

int StreamDock293V3::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDock293V3::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDock293V3::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDock293V3::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_15, DeviceKeyMaps::REMAP_15_COUNT, logical_key);
}

InputEvent StreamDock293V3::decode_input_event(int hardware_code, int state) {
    return DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_15, DeviceKeyMaps::REMAP_15_COUNT, hardware_code, state);
}

ImageFormat StreamDock293V3::key_image_format() const {
    return {112, 112, ImageFileFormat::JPEG, 180, false, false};
}

ImageFormat StreamDock293V3::touchscreen_image_format() const {
    return {800, 480, ImageFileFormat::JPEG, 180, false, false};
}
