#include "StreamDockN3.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDockN3::set_device() {
    configureReport1025(transport);
    feature_option.deviceType = device_type::dock_n3;
}

int StreamDockN3::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockN3::set_key_imageData(int key, const uint8_t *data, size_t length) {
    if (key < 1 || key > 9) {
        return -1;
    }
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockN3::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockN3::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_N3_9, DeviceKeyMaps::REMAP_N3_9_COUNT, logical_key);
}

InputEvent StreamDockN3::decode_input_event(int hardware_code, int state) {
    InputEvent event = DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_N3_9, DeviceKeyMaps::REMAP_N3_9_COUNT, hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    static const DeviceKeyMaps::KnobRotateEntry rotate_map[] = {
        {0x90, KnobId::KNOB_1, Direction::LEFT}, {0x91, KnobId::KNOB_1, Direction::RIGHT},
        {0x60, KnobId::KNOB_2, Direction::LEFT}, {0x61, KnobId::KNOB_2, Direction::RIGHT},
        {0x50, KnobId::KNOB_3, Direction::LEFT}, {0x51, KnobId::KNOB_3, Direction::RIGHT},
    };
    event = DeviceKeyMaps::decodeKnobRotate(rotate_map, sizeof(rotate_map) / sizeof(rotate_map[0]), hardware_code);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    static const DeviceKeyMaps::KnobPressEntry press_map[] = {
        {0x33, KnobId::KNOB_1}, {0x34, KnobId::KNOB_2}, {0x35, KnobId::KNOB_3},
    };
    return DeviceKeyMaps::decodeKnobPress(press_map, sizeof(press_map) / sizeof(press_map[0]), hardware_code, state);
}

ImageFormat StreamDockN3::key_image_format() const {
    return {64, 64, ImageFileFormat::JPEG, -90, false, false};
}

ImageFormat StreamDockN3::touchscreen_image_format() const {
    return {320, 240, ImageFileFormat::JPEG, -90, false, false};
}
