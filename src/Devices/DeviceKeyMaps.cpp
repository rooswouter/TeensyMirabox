#include "DeviceKeyMaps.h"

namespace DeviceKeyMaps {

int normalizeState(int state) {
    return state == 0x01 ? 1 : 0;
}

int getImageKey(const KeyMapEntry *map, size_t count, ButtonKey logical_key) {
    for (size_t i = 0; i < count; ++i) {
        if (map[i].logical == logical_key) {
            return map[i].hardware;
        }
    }
    return -1;
}

bool hwToLogical(const KeyMapEntry *map, size_t count, int hardware_code, ButtonKey &logical_key) {
    for (size_t i = 0; i < count; ++i) {
        if (map[i].hardware == hardware_code) {
            logical_key = map[i].logical;
            return true;
        }
    }
    return false;
}

InputEvent decodeButtonMap(const KeyMapEntry *map, size_t count, int hardware_code, int state) {
    ButtonKey logical_key;
    if (hwToLogical(map, count, hardware_code, logical_key)) {
        return InputEvent::button(logical_key, normalizeState(state));
    }
    return InputEvent::unknown();
}

InputEvent decodeKnobRotate(const KnobRotateEntry *map, size_t count, int hardware_code) {
    for (size_t i = 0; i < count; ++i) {
        if (map[i].hardware == hardware_code) {
            return InputEvent::knobRotate(map[i].knob, map[i].direction);
        }
    }
    return InputEvent::unknown();
}

InputEvent decodeKnobPress(const KnobPressEntry *map, size_t count, int hardware_code, int state) {
    for (size_t i = 0; i < count; ++i) {
        if (map[i].hardware == hardware_code) {
            return InputEvent::knobPress(map[i].knob, normalizeState(state));
        }
    }
    return InputEvent::unknown();
}

const KeyMapEntry REMAP_15[] = {
    {ButtonKey::BTN_1, 11}, {ButtonKey::BTN_2, 12}, {ButtonKey::BTN_3, 13},
    {ButtonKey::BTN_4, 14}, {ButtonKey::BTN_5, 15}, {ButtonKey::BTN_6, 6},
    {ButtonKey::BTN_7, 7}, {ButtonKey::BTN_8, 8}, {ButtonKey::BTN_9, 9},
    {ButtonKey::BTN_10, 10}, {ButtonKey::BTN_11, 1}, {ButtonKey::BTN_12, 2},
    {ButtonKey::BTN_13, 3}, {ButtonKey::BTN_14, 4}, {ButtonKey::BTN_15, 5},
};
const size_t REMAP_15_COUNT = sizeof(REMAP_15) / sizeof(REMAP_15[0]);

const KeyMapEntry REMAP_293S_18[] = {
    {ButtonKey::BTN_1, 13}, {ButtonKey::BTN_2, 10}, {ButtonKey::BTN_3, 7},
    {ButtonKey::BTN_4, 4}, {ButtonKey::BTN_5, 1}, {ButtonKey::BTN_6, 14},
    {ButtonKey::BTN_7, 11}, {ButtonKey::BTN_8, 8}, {ButtonKey::BTN_9, 5},
    {ButtonKey::BTN_10, 2}, {ButtonKey::BTN_11, 15}, {ButtonKey::BTN_12, 12},
    {ButtonKey::BTN_13, 9}, {ButtonKey::BTN_14, 6}, {ButtonKey::BTN_15, 3},
    {ButtonKey::BTN_16, 16}, {ButtonKey::BTN_17, 17}, {ButtonKey::BTN_18, 18},
};
const size_t REMAP_293S_18_COUNT = sizeof(REMAP_293S_18) / sizeof(REMAP_293S_18[0]);

const KeyMapEntry REMAP_N4_14[] = {
    {ButtonKey::BTN_1, 11}, {ButtonKey::BTN_2, 12}, {ButtonKey::BTN_3, 13},
    {ButtonKey::BTN_4, 14}, {ButtonKey::BTN_5, 15}, {ButtonKey::BTN_6, 6},
    {ButtonKey::BTN_7, 7}, {ButtonKey::BTN_8, 8}, {ButtonKey::BTN_9, 9},
    {ButtonKey::BTN_10, 10}, {ButtonKey::BTN_11, 1}, {ButtonKey::BTN_12, 2},
    {ButtonKey::BTN_13, 3}, {ButtonKey::BTN_14, 4},
};
const size_t REMAP_N4_14_COUNT = sizeof(REMAP_N4_14) / sizeof(REMAP_N4_14[0]);

const KeyMapEntry REMAP_N4PRO_15[] = {
    {ButtonKey::BTN_1, 11}, {ButtonKey::BTN_2, 12}, {ButtonKey::BTN_3, 13},
    {ButtonKey::BTN_4, 14}, {ButtonKey::BTN_5, 15}, {ButtonKey::BTN_6, 6},
    {ButtonKey::BTN_7, 7}, {ButtonKey::BTN_8, 8}, {ButtonKey::BTN_9, 9},
    {ButtonKey::BTN_10, 10}, {ButtonKey::BTN_11, 1}, {ButtonKey::BTN_12, 2},
    {ButtonKey::BTN_13, 3}, {ButtonKey::BTN_14, 4}, {ButtonKey::BTN_15, 5},
};
const size_t REMAP_N4PRO_15_COUNT = sizeof(REMAP_N4PRO_15) / sizeof(REMAP_N4PRO_15[0]);

const KeyMapEntry REMAP_K1PRO[] = {
    {ButtonKey::BTN_1, 0x05}, {ButtonKey::BTN_2, 0x03}, {ButtonKey::BTN_3, 0x01},
    {ButtonKey::BTN_4, 0x06}, {ButtonKey::BTN_5, 0x04}, {ButtonKey::BTN_6, 0x02},
};
const size_t REMAP_K1PRO_COUNT = sizeof(REMAP_K1PRO) / sizeof(REMAP_K1PRO[0]);

const KeyMapEntry REMAP_XL_32[] = {
    {ButtonKey::BTN_1, 25}, {ButtonKey::BTN_2, 26}, {ButtonKey::BTN_3, 27},
    {ButtonKey::BTN_4, 28}, {ButtonKey::BTN_5, 29}, {ButtonKey::BTN_6, 30},
    {ButtonKey::BTN_7, 31}, {ButtonKey::BTN_8, 32}, {ButtonKey::BTN_9, 17},
    {ButtonKey::BTN_10, 18}, {ButtonKey::BTN_11, 19}, {ButtonKey::BTN_12, 20},
    {ButtonKey::BTN_13, 21}, {ButtonKey::BTN_14, 22}, {ButtonKey::BTN_15, 23},
    {ButtonKey::BTN_16, 24}, {ButtonKey::BTN_17, 9}, {ButtonKey::BTN_18, 10},
    {ButtonKey::BTN_19, 11}, {ButtonKey::BTN_20, 12}, {ButtonKey::BTN_21, 13},
    {ButtonKey::BTN_22, 14}, {ButtonKey::BTN_23, 15}, {ButtonKey::BTN_24, 16},
    {ButtonKey::BTN_25, 1}, {ButtonKey::BTN_26, 2}, {ButtonKey::BTN_27, 3},
    {ButtonKey::BTN_28, 4}, {ButtonKey::BTN_29, 5}, {ButtonKey::BTN_30, 6},
    {ButtonKey::BTN_31, 7}, {ButtonKey::BTN_32, 8},
};
const size_t REMAP_XL_32_COUNT = sizeof(REMAP_XL_32) / sizeof(REMAP_XL_32[0]);

const KeyMapEntry REMAP_MINI_6[] = {
    {ButtonKey::BTN_1, 1}, {ButtonKey::BTN_2, 2}, {ButtonKey::BTN_3, 3},
    {ButtonKey::BTN_4, 4}, {ButtonKey::BTN_5, 5}, {ButtonKey::BTN_6, 6},
};
const size_t REMAP_MINI_6_COUNT = sizeof(REMAP_MINI_6) / sizeof(REMAP_MINI_6[0]);

const KeyMapEntry REMAP_N3_9[] = {
    {ButtonKey::BTN_1, 1}, {ButtonKey::BTN_2, 2}, {ButtonKey::BTN_3, 3},
    {ButtonKey::BTN_4, 4}, {ButtonKey::BTN_5, 5}, {ButtonKey::BTN_6, 6},
    {ButtonKey::BTN_7, 0x25}, {ButtonKey::BTN_8, 0x30}, {ButtonKey::BTN_9, 0x31},
};
const size_t REMAP_N3_9_COUNT = sizeof(REMAP_N3_9) / sizeof(REMAP_N3_9[0]);

const KeyMapEntry REMAP_N1_17[] = {
    {ButtonKey::BTN_1, 1}, {ButtonKey::BTN_2, 2}, {ButtonKey::BTN_3, 3},
    {ButtonKey::BTN_4, 4}, {ButtonKey::BTN_5, 5}, {ButtonKey::BTN_6, 6},
    {ButtonKey::BTN_7, 7}, {ButtonKey::BTN_8, 8}, {ButtonKey::BTN_9, 9},
    {ButtonKey::BTN_10, 10}, {ButtonKey::BTN_11, 11}, {ButtonKey::BTN_12, 12},
    {ButtonKey::BTN_13, 13}, {ButtonKey::BTN_14, 14}, {ButtonKey::BTN_15, 15},
    {ButtonKey::BTN_16, 0x1E}, {ButtonKey::BTN_17, 0x1F},
};
const size_t REMAP_N1_17_COUNT = sizeof(REMAP_N1_17) / sizeof(REMAP_N1_17[0]);

const KeyMapEntry REMAP_M18_18[] = {
    {ButtonKey::BTN_1, 11}, {ButtonKey::BTN_2, 12}, {ButtonKey::BTN_3, 13},
    {ButtonKey::BTN_4, 14}, {ButtonKey::BTN_5, 15}, {ButtonKey::BTN_6, 6},
    {ButtonKey::BTN_7, 7}, {ButtonKey::BTN_8, 8}, {ButtonKey::BTN_9, 9},
    {ButtonKey::BTN_10, 10}, {ButtonKey::BTN_11, 1}, {ButtonKey::BTN_12, 2},
    {ButtonKey::BTN_13, 3}, {ButtonKey::BTN_14, 4}, {ButtonKey::BTN_15, 5},
    {ButtonKey::BTN_16, 0x25}, {ButtonKey::BTN_17, 0x30}, {ButtonKey::BTN_18, 0x31},
};
const size_t REMAP_M18_18_COUNT = sizeof(REMAP_M18_18) / sizeof(REMAP_M18_18[0]);

} // namespace DeviceKeyMaps
