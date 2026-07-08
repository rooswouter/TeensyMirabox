#include "K1Pro.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

void K1Pro::set_device() {
    transport.setReportSize(513, 1024, 0);
    transport.setReportId(0x04);
    feature_option.deviceType = device_type::k1pro;
}

int K1Pro::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int K1Pro::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int K1Pro::set_touchscreen_image(const uint8_t *data, size_t length) {
    (void)data;
    (void)length;
    return 0;
}

int K1Pro::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_K1PRO, DeviceKeyMaps::REMAP_K1PRO_COUNT, logical_key);
}

InputEvent K1Pro::decode_input_event(int hardware_code, int state) {
    InputEvent event = DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_K1PRO, DeviceKeyMaps::REMAP_K1PRO_COUNT, hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    static const DeviceKeyMaps::KnobPressEntry press_map[] = {
        {0x25, KnobId::KNOB_1}, {0x30, KnobId::KNOB_2}, {0x31, KnobId::KNOB_3},
    };
    event = DeviceKeyMaps::decodeKnobPress(press_map, sizeof(press_map) / sizeof(press_map[0]), hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    static const DeviceKeyMaps::KnobRotateEntry rotate_map[] = {
        {0x50, KnobId::KNOB_1, Direction::LEFT}, {0x51, KnobId::KNOB_1, Direction::RIGHT},
        {0x60, KnobId::KNOB_2, Direction::LEFT}, {0x61, KnobId::KNOB_2, Direction::RIGHT},
        {0x90, KnobId::KNOB_3, Direction::LEFT}, {0x91, KnobId::KNOB_3, Direction::RIGHT},
    };
    return DeviceKeyMaps::decodeKnobRotate(rotate_map, sizeof(rotate_map) / sizeof(rotate_map[0]), hardware_code);
}

DeviceConfigEvent K1Pro::decode_device_config_event(const uint8_t *data, size_t length) {
    return DeviceConfigEvent(data, length);
}

ImageFormat K1Pro::key_image_format() const {
    return {64, 64, ImageFileFormat::JPEG, -90, false, false};
}

ImageFormat K1Pro::touchscreen_image_format() const {
    return {800, 480, ImageFileFormat::JPEG, 180, false, false};
}

void K1Pro::set_keyboard_backlight_brightness(int brightness) {
    transport.setKeyboardBacklightBrightness(brightness);
}

void K1Pro::set_keyboard_lighting_effects(int effect) {
    if (effect == 0) {
        set_keyboard_lighting_speed(0);
    }
    transport.setKeyboardLightingEffects(effect);
}

void K1Pro::set_keyboard_lighting_speed(int speed) {
    transport.setKeyboardLightingSpeed(speed);
}

void K1Pro::set_keyboard_rgb_backlight(int red, int green, int blue) {
    transport.setKeyboardRgbBacklight(red, green, blue);
}

void K1Pro::keyboard_os_mode_switch(int os_mode) {
    transport.keyboardOsModeSwitch(os_mode);
}

void K1Pro::keyboard_mode(int mode) {
    if (mode == 0) {
        transport.notifyDisconnected();
        Serial.println("K1Pro: keyboard_mode: 0");
    } else if (mode == 1) {
        Serial.println("K1Pro: keyboard_mode: 1");
        init();
        clearAllIcon();
        refresh();
    }
}
