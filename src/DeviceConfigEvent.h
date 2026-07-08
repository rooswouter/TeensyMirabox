#pragma once

#include <cstdint>
#include <string>

class LedInfo {
public:
    int mode = 0;
    int speed = 0;
    int brightness = 0;
    int flag = 0;
    int hsv[3] = {0, 0, 0};
    int base_hs[2] = {0, 0};
};

class DeviceConfigEvent {
public:
    std::string version;
    std::string os;
    int scr = 0;
    int style = 0;
    std::string Stream_Dock;
    LedInfo led_info;
    bool valid = false;

    DeviceConfigEvent(const uint8_t *buffer, uint32_t length);
};
