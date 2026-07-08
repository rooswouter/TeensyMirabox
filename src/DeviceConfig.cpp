#include "DeviceConfig.h"

namespace {

uint8_t encodeTriState(bool is_set, bool value) {
    if (!is_set) {
        return CONFIG_DEFAULT;
    }
    return value ? CONFIG_ON : CONFIG_OFF;
}

} // namespace

std::vector<uint8_t> StreamDockXLConfig::to_bytes() const {
    return {encodeTriState(led_follow_key_light_set, led_follow_key_light)};
}

void StreamDockXLConfig::reset() {
    led_follow_key_light_set = false;
    led_follow_key_light = false;
}

std::vector<uint8_t> StreamDockN4ProConfig::to_bytes() const {
    return {
        encodeTriState(led_follow_key_light_set, led_follow_key_light),
        encodeTriState(key_light_on_disconnect_set, key_light_on_disconnect),
        encodeTriState(check_usb_power_set, check_usb_power),
        encodeTriState(enable_vibration_set, enable_vibration),
        encodeTriState(reset_usb_report_set, reset_usb_report),
        encodeTriState(enable_boot_video_set, enable_boot_video),
    };
}

void StreamDockN4ProConfig::reset() {
    led_follow_key_light_set = false;
    key_light_on_disconnect_set = false;
    check_usb_power_set = false;
    enable_vibration_set = false;
    reset_usb_report_set = false;
    enable_boot_video_set = false;
}
