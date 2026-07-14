#include "KeyboardEvent.h"

KeyboardEvent::KeyboardEvent(const uint8_t *data, size_t length) {
    if (length != 8) {
        return;
    }
    memcpy(keycode, data, length);
}