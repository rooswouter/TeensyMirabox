#pragma once

#include <cstdint>

/**
 * @file FeatrueOption.h
 * @brief Device model identifier and capability flags.
 */

/** @brief Identifies the concrete StreamDock product type. */
enum class device_type : uint8_t {
    dock_universal = 0,
    dock_293 = 1,
    dock_293v3 = 2,
    dock_293s = 3,
    dock_293sv3 = 4,
    dock_m3 = 5,
    dock_m18 = 6,
    dock_n1 = 7,
    dock_n3 = 8,
    dock_n4 = 9,
    dock_n4pro = 10,
    dock_xl = 11,
    k1pro = 12,
    dock_mini = 13,
};

/**
 * @class FeatrueOption
 * @brief Runtime capability flags set by each device in `set_device()`.
 */
class FeatrueOption {
public:
    bool hasRGBLed = false;
    int ledCounts = 0;
    bool supportConfig = false;
    bool supportKeyGif = true;
    bool supportBackgroundGif = false;
    device_type deviceType = device_type::dock_universal;
    bool support_single_led_color = false;
};
