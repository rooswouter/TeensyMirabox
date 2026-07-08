#pragma once

#include "StreamDock.h"
#include "../DeviceConfig.h"

class StreamDockN4Pro : public StreamDock {
public:
    static constexpr int KEY_COUNT = 15;

    StreamDockN4Pro(LibUSBHIDAPI &transport, const HidDeviceInfo &dev_info);

    int key_count() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
    ImageFormat secondscreen_image_format() const;

    InputEvent decode_touch_bar_event(const uint8_t *data, size_t length) const;
    void set_touch_bar_callback(TouchscreenCallback callback);
    int set_seondscreen_image(int key, const uint8_t *data, size_t length);
    int set_frame_background(const uint8_t *data, size_t length, int width, int height);

protected:
    void handle_raw_read(const uint8_t *data, size_t length) override;
};

class StreamDockN1 : public StreamDock {
public:
    static constexpr int KEY_COUNT = 20;

    enum class DeviceMode : uint8_t {
        KEYBOARD = 0,
        CALCULATOR = 1,
        DOCK = 2,
    };

    enum class SkinMode : uint8_t {
        KEYBOARD = 0x11,
        KEYBOARD_LOCK = 0x1F,
        CALCULATOR = 0xFF,
    };

    enum class SkinStatus : uint8_t {
        PRESS = 0,
        RELEASE = 1,
    };

    using StreamDock::StreamDock;

    bool open() override;
    int key_count() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;
    ImageFormat secondscreen_image_format() const;

    void switch_mode(DeviceMode mode);
    void change_page(int page);
    int set_n1_skin_bitmap(const uint8_t *png_data, size_t png_len, int skin_mode, int skin_page, int skin_status, int key_index);
};

int extract_last_number(const char *code);
