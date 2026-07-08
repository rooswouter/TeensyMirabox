#include "StreamDockMini.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void StreamDockM3::set_device() {
    configureReport1025(transport);
    feature_option.supportBackgroundGif = true;
    feature_option.deviceType = device_type::dock_m3;
}

int StreamDockM3::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockM3::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockM3::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockM3::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_15, DeviceKeyMaps::REMAP_15_COUNT, logical_key);
}

InputEvent StreamDockM3::decode_input_event(int hardware_code, int state) {
    static const DeviceKeyMaps::KnobRotateEntry rotate_map[] = {
        {0x50, KnobId::KNOB_1, Direction::LEFT}, {0x51, KnobId::KNOB_1, Direction::RIGHT},
        {0x90, KnobId::KNOB_2, Direction::LEFT}, {0x91, KnobId::KNOB_2, Direction::RIGHT},
        {0xA0, KnobId::KNOB_3, Direction::LEFT}, {0xA1, KnobId::KNOB_3, Direction::RIGHT},
    };
    InputEvent event = DeviceKeyMaps::decodeKnobRotate(rotate_map, sizeof(rotate_map) / sizeof(rotate_map[0]), hardware_code);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    event = DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_15, DeviceKeyMaps::REMAP_15_COUNT, hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    static const DeviceKeyMaps::KnobPressEntry press_map[] = {
        {0x35, KnobId::KNOB_1}, {0x33, KnobId::KNOB_2}, {0x37, KnobId::KNOB_3},
    };
    return DeviceKeyMaps::decodeKnobPress(press_map, sizeof(press_map) / sizeof(press_map[0]), hardware_code, state);
}

ImageFormat StreamDockM3::key_image_format() const {
    return {96, 96, ImageFileFormat::PNG, 90, false, false};
}

ImageFormat StreamDockM3::touchscreen_image_format() const {
    return {854, 480, ImageFileFormat::JPEG, 90, false, false};
}

void StreamDockM3::magnetic_calibration() {
    transport.magneticCalibration();
}

int StreamDockM3::set_frame_background(const uint8_t *data, size_t length, int width, int height) {
    transport.setBackgroundFrameStream(data, length, width, height, 0, 0, 0);
    return 0;
}
