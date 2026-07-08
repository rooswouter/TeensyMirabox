#include "StreamDockN3.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDockN4::set_device() {
    configureReport1025(transport);
    feature_option.deviceType = device_type::dock_n4;
}

int StreamDockN4::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockN4::set_key_imageData(int key, const uint8_t *data, size_t length) {
    if (key >= 11 && key <= 14) {
        return set_seondscreen_image(key, data, length);
    }
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockN4::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockN4::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_N4_14, DeviceKeyMaps::REMAP_N4_14_COUNT, logical_key);
}

InputEvent StreamDockN4::decode_input_event(int hardware_code, int state) {
    return DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_N4_14, DeviceKeyMaps::REMAP_N4_14_COUNT, hardware_code, state);
}

ImageFormat StreamDockN4::key_image_format() const {
    return {112, 112, ImageFileFormat::JPEG, 180, false, false};
}

ImageFormat StreamDockN4::touchscreen_image_format() const {
    return {800, 480, ImageFileFormat::JPEG, 180, false, false};
}

ImageFormat StreamDockN4::secondscreen_image_format() const {
    return {176, 112, ImageFileFormat::JPEG, 180, false, false};
}

int StreamDockN4::set_seondscreen_image(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}
