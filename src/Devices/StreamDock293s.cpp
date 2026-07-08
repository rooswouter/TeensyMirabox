#include "StreamDock293.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDock293s::set_device() {
    configureReport513(transport);
    feature_option.deviceType = device_type::dock_293s;
}

int StreamDock293s::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDock293s::set_key_imageData(int key, const uint8_t *data, size_t length) {
    if (key < 1 || key > KEY_COUNT) {
        return -1;
    }
    const int hardware_key = get_image_key(static_cast<ButtonKey>(key));
    if (hardware_key < 0) {
        return -1;
    }
    transport.setKeyImageStream(data, length, hardware_key);
    return 0;
}

int StreamDock293s::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundBitmap(data, length);
    return 0;
}

int StreamDock293s::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_293S_18, DeviceKeyMaps::REMAP_293S_18_COUNT, logical_key);
}

InputEvent StreamDock293s::decode_input_event(int hardware_code, int state) {
    return DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_293S_18, DeviceKeyMaps::REMAP_293S_18_COUNT, hardware_code, state);
}

ImageFormat StreamDock293s::key_image_format() const {
    return {85, 85, ImageFileFormat::JPEG, 90, false, false};
}

ImageFormat StreamDock293s::touchscreen_image_format() const {
    return {854, 480, ImageFileFormat::JPEG, 0, true, false};
}

ImageFormat StreamDock293s::secondscreen_image_format() const {
    return {80, 80, ImageFileFormat::JPEG, 90, false, false};
}
