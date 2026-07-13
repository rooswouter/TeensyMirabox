#pragma once

#include "StreamDock.h"

/**
 * @file K1Pro.h
 * @brief Driver for the Mirabox K1 Pro (6 LCD keys + 3 knobs + keyboard).
 */

/**
 * @class K1Pro
 * @brief K1 Pro keyboard dock with SDK mode for button/knob events.
 *
 * Requires `keyboard_mode(1)` before button and knob callbacks are delivered.
 * Key images: 64×64 JPEG, −90° rotation.
 */
class K1Pro : public StreamDock {
public:
    static constexpr int KEY_COUNT = 6;

    using StreamDock::StreamDock;

    int image_keys() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    DeviceConfigEvent decode_device_config_event(const uint8_t *data, size_t length) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;

    /** @brief Set keyboard backlight brightness (0–100). */
    void set_keyboard_backlight_brightness(int brightness);

    /** @brief Set keyboard lighting effect index. */
    void set_keyboard_lighting_effects(int effect);

    /** @brief Set keyboard lighting animation speed. */
    void set_keyboard_lighting_speed(int speed);

    /** @brief Set keyboard RGB underglow color. */
    void set_keyboard_rgb_backlight(int red, int green, int blue);

    /**
     * @brief Switch keyboard OS layout.
     * @param os_mode 0 = Windows, 1 = macOS.
     */
    void keyboard_os_mode_switch(int os_mode);

    /**
     * @brief Switch between native keyboard and SDK (program) mode.
     * @param mode 0 = native keyboard, 1 = SDK (enables LCD key/knob callbacks).
     */
    void keyboard_mode(int mode);
};
