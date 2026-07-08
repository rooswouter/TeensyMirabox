#pragma once

#include <cstdint>
#include <vector>

static constexpr uint8_t CONFIG_DEFAULT = 0x11;
static constexpr uint8_t CONFIG_ON = 0x11;
static constexpr uint8_t CONFIG_OFF = 0xFF;

class DeviceConfig {
public:
    virtual ~DeviceConfig() = default;
    virtual std::vector<uint8_t> to_bytes() const = 0;
    virtual void reset() = 0;
};

class StreamDockXLConfig : public DeviceConfig {
public:
    bool led_follow_key_light_set = false;
    bool led_follow_key_light = false;

    std::vector<uint8_t> to_bytes() const override;
    void reset() override;
};

class StreamDockN4ProConfig : public DeviceConfig {
public:
    bool led_follow_key_light_set = false;
    bool led_follow_key_light = false;
    bool key_light_on_disconnect_set = false;
    bool key_light_on_disconnect = false;
    bool check_usb_power_set = false;
    bool check_usb_power = false;
    bool enable_vibration_set = false;
    bool enable_vibration = false;
    bool reset_usb_report_set = false;
    bool reset_usb_report = false;
    bool enable_boot_video_set = false;
    bool enable_boot_video = false;

    std::vector<uint8_t> to_bytes() const override;
    void reset() override;
};
