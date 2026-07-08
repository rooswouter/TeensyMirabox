#include "StreamDockMini.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDockM18::set_device() {
    configureReport1025(transport);
    feature_option.hasRGBLed = true;
    feature_option.ledCounts = 24;
    feature_option.deviceType = device_type::dock_m18;
}

int StreamDockM18::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockM18::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockM18::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockM18::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_M18_18, DeviceKeyMaps::REMAP_M18_18_COUNT, logical_key);
}

InputEvent StreamDockM18::decode_input_event(int hardware_code, int state) {
    return DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_M18_18, DeviceKeyMaps::REMAP_M18_18_COUNT, hardware_code, state);
}

ImageFormat StreamDockM18::key_image_format() const {
    return {64, 64, ImageFileFormat::JPEG, 0, false, false};
}

ImageFormat StreamDockM18::touchscreen_image_format() const {
    return {480, 272, ImageFileFormat::JPEG, 0, false, false};
}
