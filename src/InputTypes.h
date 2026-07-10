#pragma once

#include <cstdint>
#include <cstddef>

enum class EventType {
    BUTTON,
    KNOB_ROTATE,
    KNOB_PRESS,
    SWIPE,
    TOUCH_POINT,
    DIP_SWITCH,
    UNKNOWN,
};

// Teensyduino defines KEY_0..KEY_9 in keylayouts.h as USB keycodes.
#if defined(KEY_0)
#undef KEY_0
#endif
#if defined(KEY_1)
#undef KEY_1
#endif
#if defined(KEY_2)
#undef KEY_2
#endif
#if defined(KEY_3)
#undef KEY_3
#endif
#if defined(KEY_4)
#undef KEY_4
#endif
#if defined(KEY_5)
#undef KEY_5
#endif
#if defined(KEY_6)
#undef KEY_6
#endif
#if defined(KEY_7)
#undef KEY_7
#endif
#if defined(KEY_8)
#undef KEY_8
#endif
#if defined(KEY_9)
#undef KEY_9
#endif

// NOTE: Teensyduino's core defines KEY_0..KEY_9 as macros (USB keycodes).
// Using BTN_* avoids macro collisions in user sketches and core headers.
enum class ButtonKey : int {
    BTN_1 = 1,
    BTN_2,
    BTN_3,
    BTN_4,
    BTN_5,
    BTN_6,
    BTN_7,
    BTN_8,
    BTN_9,
    BTN_10,
    BTN_11,
    BTN_12,
    BTN_13,
    BTN_14,
    BTN_15,
    BTN_16,
    BTN_17,
    BTN_18,
    BTN_19,
    BTN_20,
    BTN_21,
    BTN_22,
    BTN_23,
    BTN_24,
    BTN_25,
    BTN_26,
    BTN_27,
    BTN_28,
    BTN_29,
    BTN_30,
    BTN_31,
    BTN_32,
};

enum class KnobId {
    KNOB_1,
    KNOB_2,
    KNOB_3,
    KNOB_4,
};

enum class DIPSwitchId {
    DIP_1,
    DIP_2,
};

enum class Direction {
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

struct InputEvent {
    EventType event_type = EventType::UNKNOWN;
    ButtonKey key = ButtonKey::BTN_1;
    bool has_key = false;
    KnobId knob_id = KnobId::KNOB_1;
    bool has_knob_id = false;
    DIPSwitchId dip_id = DIPSwitchId::DIP_1;
    bool has_dip_id = false;
    Direction direction = Direction::LEFT;
    bool has_direction = false;
    int state = 0;
    int x = 0;
    int y = 0;
    bool has_touch = false;
    const uint8_t *raw_data = nullptr;
    size_t raw_data_len = 0;

    static InputEvent unknown() {
        return InputEvent{};
    }

    static InputEvent button(ButtonKey key, int state) {
        InputEvent event;
        event.event_type = EventType::BUTTON;
        event.key = key;
        event.has_key = true;
        event.state = state;
        return event;
    }

    static InputEvent knobPress(KnobId knob_id, int state) {
        InputEvent event;
        event.event_type = EventType::KNOB_PRESS;
        event.knob_id = knob_id;
        event.has_knob_id = true;
        event.state = state;
        return event;
    }

    static InputEvent knobRotate(KnobId knob_id, Direction direction) {
        InputEvent event;
        event.event_type = EventType::KNOB_ROTATE;
        event.knob_id = knob_id;
        event.has_knob_id = true;
        event.direction = direction;
        event.has_direction = true;
        return event;
    }

    static InputEvent swipe(Direction direction) {
        InputEvent event;
        event.event_type = EventType::SWIPE;
        event.direction = direction;
        event.has_direction = true;
        return event;
    }

    static InputEvent dipSwitch(DIPSwitchId dip_id, int state, Direction direction, bool has_direction) {
        InputEvent event;
        event.event_type = EventType::DIP_SWITCH;
        event.dip_id = dip_id;
        event.has_dip_id = true;
        event.state = state;
        event.direction = direction;
        event.has_direction = has_direction;
        return event;
    }

    static InputEvent touchPoint(int x, int y, const uint8_t *raw_data, size_t raw_data_len) {
        InputEvent event;
        event.event_type = EventType::TOUCH_POINT;
        event.x = x;
        event.y = y;
        event.has_touch = true;
        event.raw_data = raw_data;
        event.raw_data_len = raw_data_len;
        return event;
    }
};
