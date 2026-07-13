#pragma once

#include "StreamDock.h"

/**
 * @file StreamDock293.h
 * @brief Drivers for Stream Dock 293 family devices (15- and 18-key variants).
 */

/**
 * @class StreamDock293
 * @brief Original Stream Dock 293 (15 keys).
 */
class StreamDock293 : public StreamDock {
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

/**
 * @class StreamDock293V3
 * @brief Stream Dock 293 V3 (15 keys).
 */
class StreamDock293V3 : public StreamDock {
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

/**
 * @class StreamDock293s
 * @brief Stream Dock 293S (15 main keys + 3 secondary screen keys = 18 total).
 *
 * Key images: 85×85 JPEG. Secondary keys 16–18 use 80×80 format.
 */
class StreamDock293s : public StreamDock {
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

    /** @return Image format for secondary screen keys (16–18). */
    ImageFormat secondscreen_image_format() const;
};

/**
 * @class StreamDock293sV3
 * @brief Stream Dock 293S V3 (18 keys).
 */
class StreamDock293sV3 : public StreamDock {
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

    /** @return Image format for secondary screen keys (16–18). */
    ImageFormat secondscreen_image_format() const;
};
