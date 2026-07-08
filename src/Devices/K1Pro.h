#pragma once

#include "StreamDock.h"

class K1Pro : public StreamDock {
public:
    static constexpr int KEY_COUNT = 6;

    using StreamDock::StreamDock;

    int key_count() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    DeviceConfigEvent decode_device_config_event(const uint8_t *data, size_t length) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;

    void set_keyboard_backlight_brightness(int brightness);
    void set_keyboard_lighting_effects(int effect);
    void set_keyboard_lighting_speed(int speed);
    void set_keyboard_rgb_backlight(int red, int green, int blue);
    void keyboard_os_mode_switch(int os_mode);
    void keyboard_mode(int mode);
};
