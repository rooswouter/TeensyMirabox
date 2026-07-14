#pragma once

#include <cstdint>
#include <cstring>
#include <cstdint>

class KeyboardEvent {
    public:
    KeyboardEvent(const uint8_t *data, size_t length);

    uint8_t keycode[8];
};