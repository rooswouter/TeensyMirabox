#pragma once

#include "StreamDock.h"

class StreamDockMini : public StreamDock {
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
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
    int set_frame_background(const uint8_t *data, size_t length, int width, int height);
};

class StreamDockM3 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 15;
    using StreamDock::StreamDock;
    int key_count() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
    void magnetic_calibration();
    int set_frame_background(const uint8_t *data, size_t length, int width, int height);
};

class StreamDockM18 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 15;
    using StreamDock::StreamDock;
    int key_count() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
};
