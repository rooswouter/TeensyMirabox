#pragma once

#include "StreamDock.h"

/**
 * @file StreamDockMini.h
 * @brief Drivers for Stream Dock Mini, M3, and M18 devices.
 */

/**
 * @class StreamDockMini
 * @brief Stream Dock Mini (6 keys).
 */
class StreamDockMini : public StreamDock {
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
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;

    /** @brief Upload a full or partial background frame. */
    int set_frame_background(const uint8_t *data, size_t length, int width, int height);
};

/**
 * @class StreamDockM3
 * @brief Stream Dock M3 (15 keys, magnetic calibration, background frames).
 */
class StreamDockM3 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 15;
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

    /** @brief Run magnetic encoder calibration (`CHECK`). */
    void magnetic_calibration();

    /** @brief Upload a background frame. */
    int set_frame_background(const uint8_t *data, size_t length, int width, int height);
};

/**
 * @class StreamDockM18
 * @brief Stream Dock M18 (15 keys).
 */
class StreamDockM18 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 15;
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
