#pragma once

#include "StreamDock.h"
#include "../DeviceConfig.h"

/**
 * @file StreamDockXL.h
 * @brief Driver for Stream Dock XL (32-key layout).
 */

/**
 * @class StreamDockXL
 * @brief Stream Dock XL with 32 LCD keys and optional XL configuration.
 */
class StreamDockXL : public StreamDock {
public:
    static constexpr int KEY_COUNT = 36;

    StreamDockXL(LibUSBHIDAPI &transport, const HidDeviceInfo &dev_info);

    int image_keys() const override { return KEY_COUNT; }
    void set_device() override;
    int set_brightness(int percent) override;
    int set_key_imageData(int key, const uint8_t *data, size_t length) override;
    int set_touchscreen_image(const uint8_t *data, size_t length) override;
    int get_image_key(ButtonKey logical_key) override;
    InputEvent decode_input_event(int hardware_code, int state) override;
    ImageFormat key_image_format() const override;
    ImageFormat touchscreen_image_format() const override;

    /**
     * @brief Upload a partial background frame.
     * @return 0 on success, negative on error.
     */
    int set_frame_background(const uint8_t *data, size_t length, int width, int height);
};
