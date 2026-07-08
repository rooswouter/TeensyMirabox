#include "StreamDockMini.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDockMini::set_device() {
    configureReport1025(transport);
    feature_option.hasRGBLed = true;
    feature_option.ledCounts = 12;
    feature_option.deviceType = device_type::dock_mini;
}

int StreamDockMini::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockMini::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockMini::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockMini::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_MINI_6, DeviceKeyMaps::REMAP_MINI_6_COUNT, logical_key);
}

InputEvent StreamDockMini::decode_input_event(int hardware_code, int state) {
    InputEvent event = DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_MINI_6, DeviceKeyMaps::REMAP_MINI_6_COUNT, hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    if (hardware_code == 0x24) {
        return InputEvent::dipSwitch(DIPSwitchId::DIP_1, DeviceKeyMaps::normalizeState(state), Direction::LEFT, true);
    }
    if (hardware_code == 0x26) {
        return InputEvent::dipSwitch(DIPSwitchId::DIP_1, DeviceKeyMaps::normalizeState(state), Direction::RIGHT, true);
    }
    if (hardware_code == 0x25) {
        return InputEvent::dipSwitch(DIPSwitchId::DIP_1, DeviceKeyMaps::normalizeState(state), Direction::LEFT, false);
    }
    if (hardware_code == 0x21) {
        return InputEvent::dipSwitch(DIPSwitchId::DIP_2, DeviceKeyMaps::normalizeState(state), Direction::LEFT, true);
    }
    if (hardware_code == 0x23) {
        return InputEvent::dipSwitch(DIPSwitchId::DIP_2, DeviceKeyMaps::normalizeState(state), Direction::RIGHT, true);
    }
    if (hardware_code == 0x22) {
        return InputEvent::dipSwitch(DIPSwitchId::DIP_2, DeviceKeyMaps::normalizeState(state), Direction::LEFT, false);
    }
    return InputEvent::unknown();
}

ImageFormat StreamDockMini::key_image_format() const {
    return {64, 64, ImageFileFormat::JPEG, 90, false, false};
}

ImageFormat StreamDockMini::touchscreen_image_format() const {
    return {320, 240, ImageFileFormat::JPEG, 90, false, false};
}

int StreamDockMini::set_frame_background(const uint8_t *data, size_t length, int width, int height) {
    transport.setBackgroundFrameStream(data, length, width, height, 0, 0, 0);
    return 0;
}
