#include "StreamDockN4Pro.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

StreamDockN4Pro::StreamDockN4Pro(LibUSBHIDAPI &transport, const HidDeviceInfo &dev_info)
    : StreamDock(transport, dev_info) {
    config = new StreamDockN4ProConfig();
}

void StreamDockN4Pro::set_device() {
    configureReport1025(transport);
    feature_option.hasRGBLed = true;
    feature_option.ledCounts = 4;
    feature_option.supportBackgroundGif = true;
    feature_option.supportConfig = true;
    feature_option.support_single_led_color = true;
    feature_option.deviceType = device_type::dock_n4pro;
}

int StreamDockN4Pro::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockN4Pro::set_key_imageData(int key, const uint8_t *data, size_t length) {
    if (key >= 11 && key <= 14) {
        return set_seondscreen_image(key, data, length);
    }
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockN4Pro::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockN4Pro::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_N4PRO_15, DeviceKeyMaps::REMAP_N4PRO_15_COUNT, logical_key);
}

InputEvent StreamDockN4Pro::decode_input_event(int hardware_code, int state) {
    InputEvent event = DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_N4PRO_15, DeviceKeyMaps::REMAP_N4PRO_15_COUNT, hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    ButtonKey secondary_key;
    switch (hardware_code) {
        case 0x40: secondary_key = ButtonKey::BTN_11; break;
        case 0x41: secondary_key = ButtonKey::BTN_12; break;
        case 0x42: secondary_key = ButtonKey::BTN_13; break;
        case 0x43: secondary_key = ButtonKey::BTN_14; break;
        default: secondary_key = ButtonKey::BTN_1; break;
    }
    if (hardware_code >= 0x40 && hardware_code <= 0x43) {
        return InputEvent::button(secondary_key, DeviceKeyMaps::normalizeState(state));
    }

    static const DeviceKeyMaps::KnobRotateEntry rotate_map[] = {
        {0xA0, KnobId::KNOB_1, Direction::LEFT}, {0xA1, KnobId::KNOB_1, Direction::RIGHT},
        {0x50, KnobId::KNOB_2, Direction::LEFT}, {0x51, KnobId::KNOB_2, Direction::RIGHT},
        {0x90, KnobId::KNOB_3, Direction::LEFT}, {0x91, KnobId::KNOB_3, Direction::RIGHT},
        {0x70, KnobId::KNOB_4, Direction::LEFT}, {0x71, KnobId::KNOB_4, Direction::RIGHT},
    };
    event = DeviceKeyMaps::decodeKnobRotate(rotate_map, sizeof(rotate_map) / sizeof(rotate_map[0]), hardware_code);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    static const DeviceKeyMaps::KnobPressEntry press_map[] = {
        {0x37, KnobId::KNOB_1}, {0x35, KnobId::KNOB_2}, {0x33, KnobId::KNOB_3}, {0x36, KnobId::KNOB_4},
    };
    event = DeviceKeyMaps::decodeKnobPress(press_map, sizeof(press_map) / sizeof(press_map[0]), hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    if (hardware_code == 0x38) {
        return InputEvent::swipe(Direction::LEFT);
    }
    if (hardware_code == 0x39) {
        return InputEvent::swipe(Direction::RIGHT);
    }
    return InputEvent::unknown();
}

ImageFormat StreamDockN4Pro::key_image_format() const {
    return {112, 112, ImageFileFormat::PNG, 180, false, false};
}

ImageFormat StreamDockN4Pro::touchscreen_image_format() const {
    return {800, 480, ImageFileFormat::JPEG, 180, false, false};
}

ImageFormat StreamDockN4Pro::secondscreen_image_format() const {
    return {176, 112, ImageFileFormat::PNG, 180, false, false};
}

InputEvent StreamDockN4Pro::decode_touch_bar_event(const uint8_t *data, size_t length) const {
    if (data == nullptr || length < 14) {
        return InputEvent::unknown();
    }

    const bool is_touch_packet = data[0] == 0x41 && data[1] == 0x43 && data[2] == 0x4B
        && data[4] == 0x41 && data[5] == 0x52 && data[6] == 0x58;
    if (!is_touch_packet) {
        return InputEvent::unknown();
    }

    const int x_pos = (data[10] << 8) | data[11];
    const int y_pos = (data[12] << 8) | data[13];
    return InputEvent::touchPoint(x_pos, y_pos, data, length);
}

void StreamDockN4Pro::set_touch_bar_callback(TouchscreenCallback callback) {
    set_touchscreen_callback(callback);
}

int StreamDockN4Pro::set_seondscreen_image(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockN4Pro::set_frame_background(const uint8_t *data, size_t length, int width, int height) {
    transport.setBackgroundFrameStream(data, length, width, height, 0, 0, 0);
    return 0;
}

void StreamDockN4Pro::handle_raw_read(const uint8_t *data, size_t length) {
    StreamDock::handle_raw_read(data, length);
    const InputEvent event = decode_touch_bar_event(data, length);
    if (event.event_type == EventType::TOUCH_POINT) {
        dispatch_touchscreen_event(event);
    }
}
