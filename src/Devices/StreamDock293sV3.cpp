#include "StreamDock293.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDock293sV3::set_device() {
    configureReport1025(transport);
    feature_option.deviceType = device_type::dock_293sv3;
}

int StreamDock293sV3::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDock293sV3::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDock293sV3::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDock293sV3::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_293S_18, DeviceKeyMaps::REMAP_293S_18_COUNT, logical_key);
}

InputEvent StreamDock293sV3::decode_input_event(int hardware_code, int state) {
    return DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_293S_18, DeviceKeyMaps::REMAP_293S_18_COUNT, hardware_code, state);
}

ImageFormat StreamDock293sV3::key_image_format() const {
    return {96, 96, ImageFileFormat::JPEG, 90, false, false};
}

ImageFormat StreamDock293sV3::touchscreen_image_format() const {
    return {854, 480, ImageFileFormat::JPEG, 90, false, false};
}

ImageFormat StreamDock293sV3::secondscreen_image_format() const {
    return {80, 80, ImageFileFormat::JPEG, 90, false, false};
}
