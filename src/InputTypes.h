#pragma once

#include <cstdint>
#include <cstddef>

/**
 * @file InputTypes.h
 * @brief Unified input event types for StreamDock devices.
 */

/** @brief Category of user input reported by a device. */
enum class EventType {
    BUTTON,       /**< Physical LCD key press or release. */
    KNOB_ROTATE,  /**< Rotary encoder turned left or right. */
    KNOB_PRESS,   /**< Rotary encoder pushed in. */
    SWIPE,        /**< Touch swipe gesture. */
    TOUCH_POINT,  /**< Absolute touch coordinates. */
    DIP_SWITCH,   /**< DIP switch change. */
    UNKNOWN,      /**< Unrecognized hardware code. */
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

/**
 * @brief Logical LCD key index (1-based).
 *
 * Named BTN_* instead of KEY_* to avoid collisions with Teensyduino
 * `keylayouts.h` macros.
 */
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

/** @brief Rotary encoder identifier. */
enum class KnobId {
    KNOB_1,
    KNOB_2,
    KNOB_3,
    KNOB_4,
};

/** @brief DIP switch identifier. */
enum class DIPSwitchId {
    DIP_1,
    DIP_2,
};

/** @brief Direction for knobs, swipes, and DIP switches. */
enum class Direction {
    LEFT,
    RIGHT,
    UP,
    DOWN,
};

/**
 * @brief Normalized input event delivered to `StreamDock::KeyCallback`.
 */
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
    int state = 0;          /**< 1 = pressed / active, 0 = released. */
    int x = 0;
    int y = 0;
    bool has_touch = false;
    const uint8_t *raw_data = nullptr;
    size_t raw_data_len = 0;

    /** @return An event with `EventType::UNKNOWN`. */
    static InputEvent unknown() {
        return InputEvent{};
    }

    /**
     * @brief Build a button press/release event.
     * @param key Logical key.
     * @param state 1 = pressed, 0 = released.
     */
    static InputEvent button(ButtonKey key, int state) {
        InputEvent event;
        event.event_type = EventType::BUTTON;
        event.key = key;
        event.has_key = true;
        event.state = state;
        return event;
    }

    /** @brief Build a knob push event. */
    static InputEvent knobPress(KnobId knob_id, int state) {
        InputEvent event;
        event.event_type = EventType::KNOB_PRESS;
        event.knob_id = knob_id;
        event.has_knob_id = true;
        event.state = state;
        return event;
    }

    /** @brief Build a knob rotation event. */
    static InputEvent knobRotate(KnobId knob_id, Direction direction) {
        InputEvent event;
        event.event_type = EventType::KNOB_ROTATE;
        event.knob_id = knob_id;
        event.has_knob_id = true;
        event.direction = direction;
        event.has_direction = true;
        return event;
    }

    /** @brief Build a swipe gesture event. */
    static InputEvent swipe(Direction direction) {
        InputEvent event;
        event.event_type = EventType::SWIPE;
        event.direction = direction;
        event.has_direction = true;
        return event;
    }

    /** @brief Build a DIP switch event. */
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

    /** @brief Build a touch coordinate event. */
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
