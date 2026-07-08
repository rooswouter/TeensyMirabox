#pragma once

#include "../InputTypes.h"
#include <cstddef>
#include <cstdint>

namespace DeviceKeyMaps {

struct KeyMapEntry {
    ButtonKey logical;
    int hardware;
};

struct KnobRotateEntry {
    int hardware;
    KnobId knob;
    Direction direction;
};

struct KnobPressEntry {
    int hardware;
    KnobId knob;
};

int normalizeState(int state);
int getImageKey(const KeyMapEntry *map, size_t count, ButtonKey logical_key);
bool hwToLogical(const KeyMapEntry *map, size_t count, int hardware_code, ButtonKey &logical_key);
InputEvent decodeButtonMap(const KeyMapEntry *map, size_t count, int hardware_code, int state);
InputEvent decodeKnobRotate(const KnobRotateEntry *map, size_t count, int hardware_code);
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
