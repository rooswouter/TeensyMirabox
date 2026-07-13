#pragma once

#include "StreamDock.h"

/**
 * @file StreamDockN3.h
 * @brief Drivers for Stream Dock N3 and N4 devices.
 */

/**
 * @class StreamDockN3
 * @brief Stream Dock N3 (18 keys).
 */
class StreamDockN3 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 18;
    using StreamDock::StreamDock;
    int image_keys() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
};

/**
 * @class StreamDockN4
 * @brief Stream Dock N4 (14 keys + secondary screen keys).
 */
class StreamDockN4 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 14;
    using StreamDock::StreamDock;
    int image_keys() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;

    /** @return Image format for secondary screen keys. */
    ImageFormat secondscreen_image_format() const;

    /**
     * @brief Set image on a secondary screen key.
     * @return 0 on success, negative on error.
     */
    int set_secondscreen_image(int key, const uint8_t *data, size_t length);
};
