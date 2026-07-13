#pragma once

#include "StreamDock.h"
#include "../DeviceConfig.h"

/**
 * @file StreamDockN4Pro.h
 * @brief Drivers for Stream Dock N4 Pro and N1 devices.
 */

/**
 * @class StreamDockN4Pro
 * @brief Stream Dock N4 Pro (15 keys, touch bar, RGB LEDs, device config).
 */
class StreamDockN4Pro : public StreamDock {
public:
    static constexpr int KEY_COUNT = 15;

    StreamDockN4Pro(LibUSBHIDAPI &transport, const HidDeviceInfo &dev_info);

    int image_keys() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
    ImageFormat secondscreen_image_format() const;

    /** @brief Decode a touch-bar HID packet into an InputEvent. */
    InputEvent decode_touch_bar_event(const uint8_t *data, size_t length) const;

    /** @brief Register callback for touch-bar events. */
    void set_touch_bar_callback(TouchscreenCallback callback);

    /** @brief Set image on a secondary screen key. */
    int set_secondscreen_image(int key, const uint8_t *data, size_t length);

    /**
     * @brief Upload a partial background frame.
     * @return 0 on success, negative on error.
     */
    int set_frame_background(const uint8_t *data, size_t length, int width, int height);

protected:
    void handle_raw_read(const uint8_t *data, size_t length) override;
};

/**
 * @class StreamDockN1
 * @brief Stream Dock N1 (20 keys, multiple UI modes and skin bitmaps).
 */
class StreamDockN1 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 20;

    /** @brief N1 UI mode selector. */
    enum class DeviceMode : uint8_t {
        KEYBOARD = 0,
        CALCULATOR = 1,
        DOCK = 2,
    };

    /** @brief Skin bitmap category for N1. */
    enum class SkinMode : uint8_t {
        KEYBOARD = 0x11,
        KEYBOARD_LOCK = 0x1F,
        CALCULATOR = 0xFF,
    };

    /** @brief Skin press vs release state. */
    enum class SkinStatus : uint8_t {
        PRESS = 0,
        RELEASE = 1,
    };

    using StreamDock::StreamDock;

    bool open() override;
    int image_keys() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
    ImageFormat secondscreen_image_format() const;

    /** @brief Switch N1 between keyboard, calculator, and dock modes. */
    void switch_mode(DeviceMode mode);

    /** @brief Change the active page within the current mode. */
    void change_page(int page);

    /**
     * @brief Upload a PNG skin bitmap for N1 keys.
     * @return 0 on success, negative on error.
     */
    int set_n1_skin_bitmap(const uint8_t *png_data, size_t png_len, int skin_mode, int skin_page, int skin_status, int key_index);
};

/**
 * @brief Extract trailing digits from a string (N1 helper).
 * @param code Input string.
 * @return Parsed integer, or -1 on failure.
 */
int extract_last_number(const char *code);
