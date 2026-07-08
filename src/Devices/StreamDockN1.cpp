#include "StreamDockN4Pro.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

#include <cstring>

bool StreamDockN1::open() {
    if (!StreamDock::open()) {
        return false;
    }
    transport.changeMode(2);
    return true;
}

void StreamDockN1::set_device() {
    configureReport1025(transport);
    feature_option.deviceType = device_type::dock_n1;
}

int StreamDockN1::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockN1::set_key_imageData(int key, const uint8_t *data, size_t length) {
    if (key < 1 || key > 18) {
        return -1;
    }
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockN1::set_touchscreen_image(const uint8_t *data, size_t length) {
    if (extract_last_number(serial_number.c_str()) < 13) {
        return -1;
    }
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockN1::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_N1_17, DeviceKeyMaps::REMAP_N1_17_COUNT, logical_key);
}

InputEvent StreamDockN1::decode_input_event(int hardware_code, int state) {
    static const DeviceKeyMaps::KnobRotateEntry rotate_map[] = {
        {0x32, KnobId::KNOB_1, Direction::LEFT}, {0x33, KnobId::KNOB_1, Direction::RIGHT},
    };
    InputEvent event = DeviceKeyMaps::decodeKnobRotate(rotate_map, sizeof(rotate_map) / sizeof(rotate_map[0]), hardware_code);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    event = DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_N1_17, DeviceKeyMaps::REMAP_N1_17_COUNT, hardware_code, state);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }

    static const DeviceKeyMaps::KnobPressEntry press_map[] = {
        {0x23, KnobId::KNOB_1},
    };
    return DeviceKeyMaps::decodeKnobPress(press_map, sizeof(press_map) / sizeof(press_map[0]), hardware_code, state);
}

ImageFormat StreamDockN1::key_image_format() const {
    return {96, 96, ImageFileFormat::JPEG, 0, false, false};
}

ImageFormat StreamDockN1::touchscreen_image_format() const {
    return {480, 854, ImageFileFormat::JPEG, 0, false, false};
}

ImageFormat StreamDockN1::secondscreen_image_format() const {
    return {80, 80, ImageFileFormat::JPEG, 0, false, false};
}

void StreamDockN1::switch_mode(DeviceMode mode) {
    transport.changeMode(static_cast<int>(mode));
}

void StreamDockN1::change_page(int page) {
    transport.changePage(page);
}

int StreamDockN1::set_n1_skin_bitmap(const uint8_t *png_data, size_t png_len, int skin_mode, int skin_page, int skin_status, int key_index) {
    transport.setN1SkinBitmap(png_data, png_len, skin_mode, skin_page, skin_status, key_index);
    return 0;
}

int extract_last_number(const char *code) {
    if (code == nullptr) {
        return -1;
    }
    const char *last_dot = strrchr(code, '.');
    if (last_dot == nullptr) {
        return -1;
    }
    int value = 0;
    bool found = false;
    for (const char *p = last_dot + 1; *p != '\0'; ++p) {
        if (*p >= '0' && *p <= '9') {
            value = value * 10 + (*p - '0');
            found = true;
        } else if (found) {
            break;
        }
    }
    return found ? value : -1;
}
