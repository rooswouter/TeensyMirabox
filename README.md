# TeensyMiraBox

[![.github/workflows/ci.yml](https://github.com/rooswouter/TeensyMiraBox/actions/workflows/ci.yml/badge.svg)](https://github.com/rooswouter/TeensyMiraBox/actions/workflows/ci.yml)

Arduino library for controlling **MiraBox / StreamDock** devices from a **Teensy** board using **USB Host** (`USBHost_t36`).

This is a port of the [StreamDock Python SDK](https://github.com/MiraboxLab/StreamDock-Device-SDK) adapted for embedded use: no threads, no desktop HID stack — just `setup()` / `loop()` polling.

## Supported hardware

| Requirement | Notes |
|-------------|--------|
| **MCU** | Teensy 4.x or 3.6 recommended (USB host + enough RAM for image transfers) |
| **USB** | USB Host port wired to the StreamDock device |
| **SD card** | (optional) Built-in SD (`BUILTIN_SDCARD`) used by `set_key_image()` to load JPEG files |

### Supported devices

The library auto-detects devices by USB VID/PID and instantiates the matching driver class:

| Class | Device |Tested|
|-------|--------|--------|
| `StreamDock293` | Stream Dock 293 | ❌ |
| `StreamDock293V3` | Stream Dock 293 V3 | ❌ |
| `StreamDock293s` | Stream Dock 293S |  ✅ |
| `StreamDock293sV3` | Stream Dock 293S V3 | ❌ |
| `StreamDockN3` | Stream Dock N3 | ✅ |
| `StreamDockN4` | Stream Dock N4 | ✅ |
| `StreamDockN4Pro` | Stream Dock N4 Pro | ❌ |
| `StreamDockN1` | Stream Dock N1 | ❌ |
| `StreamDockXL` | Stream Dock XL | ❌ |
| `StreamDockM3` | Stream Dock M3 | ❌ |
| `StreamDockM18` | Stream Dock M18 | ❌ |
| `StreamDockMini` | Stream Dock Mini | ❌ |
| `K1Pro` | K1 Pro | ✅ |

## Installation

1. Copy or symlink this folder into your Arduino `libraries` directory, e.g.  
   `Documents/Arduino/libraries/MiraBox`
2. Install **Teensyduino** with the **USBHost_t36** library enabled.
3. (Optional) Install **AnimatedGIF** and **JPEGENC** for GIF support — see [Animated GIFs](#animated-gifs-optional).
4. Select a Teensy 3.6 or 4.x board and compile an example sketch.

## Quick start

The easiest path is `DeviceManager`, which handles USB enumeration, hotplug, and polling:

```cpp
#include <USBHost_t36.h>
#include "DeviceManager.h"

USBHost myusb;
DeviceManager dm(myusb);

void onAdded(StreamDock *device) {
  if (device->get_device_type() == device_type::k1pro) {
    static_cast<K1Pro *>(device)->keyboard_mode(1);  // required for K1 Pro SDK mode
  }
  device->init();
  device->set_key_callback([](StreamDock *d, const InputEvent &e) {
  });
}

void setup() {
  Serial.begin(115200);
  dm.setDeviceChangeCallback(onAdded, nullptr);
  dm.begin(true, false);   // auto_open; K1 Pro: leave auto_init false
  myusb.begin();
}

void loop() {
  dm.poll();  // runs USBHost::Task() and device->poll()
}
```

See `examples/MiraBox/Mirabox.ino` and `examples/k1pro/k1pro.ino` for complete sketches.

## Architecture

```
Sketch
  └── DeviceManager          Hotplug + polling
        └── StreamDock*        Device-specific logic (images, keys, decode)
              └── LibUSBHIDAPI HID transport (CRT protocol)
                    └── MiraBoxHIDInput   USBHost_t36 HID binding
```

### Two integration styles

1. **`DeviceManager`** (recommended) — plug-and-play detection, one `poll()` call in `loop()`.
2. **Manual** — create `USBHIDParser`, `MiraBoxHIDInput`, `LibUSBHIDAPI`, and a concrete `StreamDock` subclass yourself (see `examples/k1pro/k1pro.ino`).

## Key images

- Keys are numbered **1 … N** (not zero-based).
- Call `refresh()` after uploading images to push them to the display.
- Scaling and rotation of images is not implemented! When using `set_key_image(key, "file.jpg")`, the path is prepended
with the {device name}\key directory to read the correctly scaled and rotated file.  per device. Use convert_images.py to create these images! 
- `set_key_image(key, "file.jpg")` reads from the **SD card** root (or path relative to SD root).

## Animated GIFs (optional)

Key and background GIF animation requires the [AnimatedGIF](https://github.com/bitbank2/AnimatedGIF) and [JPEGENC](https://github.com/bitbank2/JPEGENC) libraries and a compile-time flag:

```cpp
#define WITH_ANIMATEDGIF
#include <MiraBox.h>
```

Or add `-DWITH_ANIMATEDGIF` to the Arduino compiler flags.

Without the flag, `set_key_gif()` / `set_background_gif()` return `-1` and the library does not depend on those packages. The lower-level `set_key_gif_stream()` API (pre-decoded JPEG frames) always works.

### SD card layout for GIFs

| API | SD path |
|-----|---------|
| `set_key_gif(key, "anim.gif")` | `{device_type}/key/anim.gif` |
| `set_background_gif("bg.gif")` | `{device_type}/background/bg.gif` |

Use the same device-type folder names as for JPEG key images (e.g. `StreamDockN3/key/anim.gif`). Absolute paths starting with `/` are also supported.

### GIF requirements

- Canvas size must **exactly match** the device key or background dimensions (no scaling or rotation in v1).
- Maximum file size: 512 KB (`GIF_MAX_FILE_BYTES`).
- Maximum frames: 64 (`GIF_MAX_FRAMES`).
- Maximum canvas: 480×480 (AnimatedGIF Teensy `MAX_WIDTH` limit). Background GIFs wider than 480 px are not supported.
- Key GIFs require a JPEG key format (`ImageFileFormat::JPEG`); devices that use PNG keys (e.g. XL) are not supported for key GIFs in v1.

Successful `set_key_gif()` / `set_background_gif()` calls upload frame 0 immediately and start the GIF loop automatically.

### Sharing decoded GIFs between keys or devices

Decode once, then reuse the frame buffers via `GifSharedStream` (reference counted):

```cpp
// Same device: load key 1, share to keys 2 and 3
device->set_key_gif(1, "anim.gif");
GifSharedStream shared = device->export_key_gif_stream(1);
device->set_key_gif_shared(2, shared);
device->set_key_gif_shared(3, shared);

// Across devices (same GIF dimensions)
GifSharedStream shared = deviceA->export_key_gif_stream(1);
deviceB->set_key_gif_shared(2, shared);
```

Clearing any key or calling `close()` releases one reference; frame memory is freed when the last reference is dropped.

See `examples/GifExample/GifExample.ino`.

## Input events

Register a callback with `set_key_callback()`. Events are delivered as `InputEvent` with an `EventType`:

| `EventType` | Fields used |
|-------------|-------------|
| `BUTTON` | `key`, `state` (1 = pressed, 0 = released) |
| `KNOB_PRESS` | `knob_id`, `state` |
| `KNOB_ROTATE` | `knob_id`, `direction` |
| `SWIPE` | `direction` |
| `TOUCH_POINT` | `x`, `y` |
| `DIP_SWITCH` | `dip_id`, `state`, `direction` |

Button keys use `ButtonKey::BTN_1` … `BTN_N` (not `KEY_*`, to avoid Teensy macro collisions).

## K1 Pro notes

The K1 Pro ships in **keyboard mode**. Switch to SDK mode before expecting button/knob callbacks:

```cpp
K1Pro *k1 = static_cast<K1Pro *>(device);
k1->keyboard_mode(1);
```


## Debugging HID traffic

```cpp
MiraBoxHIDInput::show_raw_data = true;       // hex dumps of IN/OUT reports
MiraBoxHIDInput::show_formated_data = true;  // parsed HID usage fields
```

Leave both `false` in production sketches.

## Examples

| Sketch | Description |
|--------|-------------|
| `examples/Mirabox/Mirabox.ino` | `DeviceManager` hotplug, images from SD, key callbacks |
| `examples/k1pro/k1pro.ino` | Manual K1 Pro bring-up with serial commands |
| `examples/GifExample/GifExample.ino` | Animated key GIF from SD (`WITH_ANIMATEDGIF`) |

## API reference

Public API documentation is in the header files (Doxygen / Arduino style):

- `DeviceManager.h` — device detection and hotplug
- `Devices/StreamDock.h` — base device class
- `Devices/K1Pro.h`, `Devices/StreamDock293.h`, … — per-model drivers
- `Transport/LibUSBHIDAPI.h` — low-level HID protocol
- `MiraBoxHIDInput.h` — USB host HID driver
- `InputTypes.h` — `InputEvent`, `EventType`, `ButtonKey`



