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
    
    // N4 Keys are not mapped the same as the images. The key logical is equal to the key hardware, so we can simple send the logical key code
    
    if(hardware_code >= 1 && hardware_code <= 14) {
        return InputEvent::button(static_cast<ButtonKey>(hardware_code), state);
    }


    static const DeviceKeyMaps::KnobPressEntry press_map[] = {
        {0x37, KnobId::KNOB_1}, {0x35, KnobId::KNOB_2}, {0x33, KnobId::KNOB_3},{0x36, KnobId::KNOB_4},
    };
    InputEvent event = DeviceKeyMaps::decodeKnobPress(press_map, sizeof(press_map) / sizeof(press_map[0]), hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
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
    switch(hardware_code) {
        case 0x38:
            return InputEvent::swipe(Direction::LEFT);
        case 0x39:
            return InputEvent::swipe(Direction::RIGHT);
        case 0xb1:
            return InputEvent::swipe(Direction::UP);
        case 0xb2:
            return InputEvent::swipe(Direction::DOWN);
    }

    return event;
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
