#pragma once

#include "../InputTypes.h"
#include <cstddef>
#include <cstdint>

/**
 * @file DeviceKeyMaps.h
 * @brief Logical-to-hardware key maps and input decoders per device model.
 */

namespace DeviceKeyMaps {

/** @brief Maps one logical ButtonKey to a firmware hardware key index. */
struct KeyMapEntry {
    ButtonKey logical;
    int hardware;
};

/** @brief Maps a hardware rotation code to knob ID and direction. */
struct KnobRotateEntry {
    int hardware;
    KnobId knob;
    Direction direction;
};

/** @brief Maps a hardware code to a knob press event. */
struct KnobPressEntry {
    int hardware;
    KnobId knob;
};

/**
 * @brief Normalize firmware state bytes to 0 (released) or 1 (pressed).
 * @param state Raw state from HID report (0x01 = press, 0x02 = release).
 */
int normalizeState(int state);

/**
 * @brief Look up hardware key index for a logical key.
 * @return Hardware key, or -1 if not in the map.
 */
int getImageKey(const KeyMapEntry *map, size_t count, ButtonKey logical_key);

/**
 * @brief Look up logical key for a hardware key code.
 * @return True if a mapping was found; `logical_key` is set on success.
 */
bool hwToLogical(const KeyMapEntry *map, size_t count, int hardware_code, ButtonKey &logical_key);

/** @brief Decode a button press/release from hardware codes. */
InputEvent decodeButtonMap(const KeyMapEntry *map, size_t count, int hardware_code, int state);

/** @brief Decode a knob rotation from a hardware code. */
InputEvent decodeKnobRotate(const KnobRotateEntry *map, size_t count, int hardware_code);

/** @brief Decode a knob press from a hardware code. */
InputEvent decodeKnobPress(const KnobPressEntry *map, size_t count, int hardware_code, int state);

extern const KeyMapEntry REMAP_15[];
extern const size_t REMAP_15_COUNT;

extern const KeyMapEntry REMAP_293S_18[];
extern const size_t REMAP_293S_18_COUNT;

extern const KeyMapEntry REMAP_N4_14[];
extern const size_t REMAP_N4_14_COUNT;

extern const KeyMapEntry REMAP_N4PRO_15[];
extern const size_t REMAP_N4PRO_15_COUNT;

extern const KeyMapEntry REMAP_K1PRO[];
extern const size_t REMAP_K1PRO_COUNT;

extern const KeyMapEntry REMAP_XL_32[];
extern const size_t REMAP_XL_32_COUNT;

extern const KeyMapEntry REMAP_MINI_6[];
extern const size_t REMAP_MINI_6_COUNT;

extern const KeyMapEntry REMAP_N3_9[];
extern const size_t REMAP_N3_9_COUNT;

extern const KeyMapEntry REMAP_N1_17[];
extern const size_t REMAP_N1_17_COUNT;

extern const KeyMapEntry REMAP_M18_18[];
extern const size_t REMAP_M18_18_COUNT;

} // namespace DeviceKeyMaps
