/*
 * K1Pro test sketch for Teensy USB Host
 *
 * Connect a K1Pro over USB host, then use the Serial Monitor (115200) to
 * exercise device features and watch button/knob/config events.
 *
 * Serial commands (type command + Enter):
 *   ?              help
 *   b <0-100>      key brightness
 *   r              refresh display
 *   c              clear all key icons
 *   h              send heartbeat
 *   kb <0-6>       keyboard backlight brightness
 *   fx <0-9>       keyboard lighting effect (0 = static)
 *   sp <0-7>       keyboard lighting speed
 *   rgb <r> <g> <b> keyboard RGB color
 *   os <0|1>       OS mode (0=Windows, 1=macOS)
 *   mode <0|1>     keyboard mode (0=native, 1=SDK/program)
 */

#include <USBHost_t36.h>
#include "MiraBoxHIDInput.h"
#include "USBDeviceInfo.h"
#include "Transport/LibUSBHIDAPI.h"
#include "Devices/Devices.h"
#include <SD.h>
#include <SPI.h>
USBHost myusb;

USBDeviceInfo dinfo(myusb);
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBHIDParser hid3(myusb);
USBHIDParser hid4(myusb);
USBHIDParser hid5(myusb);

MiraBoxHIDInput hdc1(myusb, 1);  // keyboard HID
MiraBoxHIDInput hdc2(myusb, 2);  // StreamDock control HID
LibUSBHIDAPI transport(hdc2);

HidDeviceInfo dev_info;
K1Pro device(transport, dev_info);

USBDriver *drivers[] = {&hid1, &hid2};
#define CNT_DEVICES (sizeof(drivers) / sizeof(drivers[0]))
const char *driver_names[CNT_DEVICES] = {"HID1", "HID2"};
bool driver_active[CNT_DEVICES] = {false};

USBHIDInput *hiddrivers[] = {&hdc1, &hdc2};
#define CNT_HIDDEVICES (sizeof(hiddrivers) / sizeof(hiddrivers[0]))
const char *hid_driver_names[CNT_HIDDEVICES] = {"hdc1", "hdc2"};
bool hid_driver_active[CNT_HIDDEVICES] = {false};

bool device_ready = false;

enum InitPhase : uint8_t {
  INIT_NONE,
  INIT_WAIT,
  INIT_SET_DEVICE,
  INIT_HEARTBEAT,
  INIT_WAKE,
  INIT_BRIGHTNESS,
  INIT_CLEAR,
  INIT_REFRESH,
  INIT_KB_BACKLIGHT,
  INIT_KB_FX,
  INIT_KB_RGB,
  INIT_DONE,
};

InitPhase init_phase = INIT_NONE;
unsigned long init_phase_ms = 0;
unsigned long connect_ms = 0;

static const unsigned long INIT_WAIT_MS = 500;
static const unsigned long INIT_STEP_MS = 100;

// ---------------------------------------------------------------------------
// Event callbacks
// ---------------------------------------------------------------------------

static const char *buttonKeyName(ButtonKey key) {
  switch (key) {
    case ButtonKey::BTN_1: return "BTN_1";
    case ButtonKey::BTN_2: return "BTN_2";
    case ButtonKey::BTN_3: return "BTN_3";
    case ButtonKey::BTN_4: return "BTN_4";
    case ButtonKey::BTN_5: return "BTN_5";
    case ButtonKey::BTN_6: return "BTN_6";
    default: return "?";
  }
}

static const char *knobName(KnobId knob) {
  switch (knob) {
    case KnobId::KNOB_1: return "KNOB_1";
    case KnobId::KNOB_2: return "KNOB_2";
    case KnobId::KNOB_3: return "KNOB_3";
    default: return "?";
  }
}

static const char *directionName(Direction direction) {
  return direction == Direction::LEFT ? "LEFT" : "RIGHT";
}

void onKeyEvent(StreamDock *dock, const InputEvent &event) {
  (void)dock;
  switch (event.event_type) {
    case EventType::BUTTON:
      Serial.printf("[EVENT] Button %s %s\n",
                    buttonKeyName(event.key),
                    event.state ? "pressed" : "released");
      break;
    case EventType::KNOB_PRESS:
      Serial.printf("[EVENT] %s %s\n",
                    knobName(event.knob_id),
                    event.state ? "pressed" : "released");
      break;
    case EventType::KNOB_ROTATE:
      Serial.printf("[EVENT] %s rotated %s\n",
                    knobName(event.knob_id),
                    directionName(event.direction));
      break;
    default:
      Serial.println("[EVENT] Unknown input event");
      break;
  }
}

void onConfigEvent(StreamDock *dock, const DeviceConfigEvent &config) {
  (void)dock;

  Serial.println("[CONFIG] Device configuration received:");
  Serial.printf("  version     : %s\n", config.version.c_str());
  Serial.printf("  os          : %s\n", config.os.c_str());
  Serial.printf("  scr         : %d\n", config.scr);
  Serial.printf("  style       : %d\n", config.style);
  Serial.printf("  Stream Dock : %s\n", config.Stream_Dock.c_str());
  Serial.printf("  led mode    : %d\n", config.led_info.mode);
  Serial.printf("  led speed   : %d\n", config.led_info.speed);
  Serial.printf("  led bright  : %d\n", config.led_info.brightness);
  Serial.printf("  led hsv     : [%d, %d, %d]\n",
                config.led_info.hsv[0],
                config.led_info.hsv[1],
                config.led_info.hsv[2]);
}

// Optional: enable with set_raw_read_callback(onRawRead) for HID hex dumps.
#if 0
void onRawRead(StreamDock *dock, const uint8_t *data, size_t length) {
  (void)dock;
  Serial.printf("[RAW] %u bytes: ", (unsigned)length);
  const size_t preview = length < 32 ? length : 32;
  for (size_t i = 0; i < preview; ++i) {
    Serial.printf("%02X ", data[i]);
  }
  if (length > preview) {
    Serial.print("...");
  }
  Serial.println();
}
#endif

// ---------------------------------------------------------------------------
// Device connect / disconnect
// ---------------------------------------------------------------------------

void onK1ProConnected() {
  dev_info.vendor_id = hdc2.idVendor();
  dev_info.product_id = hdc2.idProduct();
  dev_info.serial_number = hdc2.serialNumber() ? (const char *)hdc2.serialNumber() : "";

  const uint8_t *manufacturer = hdc2.manufacturer();
  const uint8_t *product = hdc2.product();
  if (manufacturer && *manufacturer) {
    dev_info.manufacturer_string = (const char *)manufacturer;
  }
  if (product && *product) {
    dev_info.product_string = (const char *)product;
  }

  Serial.printf("K1Pro HID connected %04X:%04X\n", dev_info.vendor_id, dev_info.product_id);
  if (!dev_info.manufacturer_string.empty()) {
    Serial.printf("  manufacturer: %s\n", dev_info.manufacturer_string.c_str());
  }
  if (!dev_info.product_string.empty()) {
    Serial.printf("  product: %s\n", dev_info.product_string.c_str());
  }
  if (!dev_info.serial_number.empty()) {
    Serial.printf("  serial: %s\n", dev_info.serial_number.c_str());
  }
  Serial.printf("  HID in/out report size: %u / %u\n",
                hdc2.inputReportSize(),
                hdc2.outputReportSize());

  if (!device.open()) {
    Serial.println("Failed to open K1Pro transport");
    return;
  }

  device.set_key_callback(onKeyEvent);

  init_phase = INIT_WAIT;
  connect_ms = millis();
  init_phase_ms = connect_ms;
  Serial.println("K1Pro transport open, scheduling SDK mode...");
}

void serviceDeferredInit() {
  if (init_phase == INIT_NONE || init_phase == INIT_DONE) {
    return;
  }

  const unsigned long now = millis();

  if (init_phase == INIT_WAIT) {
    if (now - connect_ms < INIT_WAIT_MS) {
      return;
    }
    init_phase = INIT_SET_DEVICE;
  } else if (now - init_phase_ms < INIT_STEP_MS) {
    return;
  }

  init_phase_ms = now;

  switch (init_phase) {
    case INIT_SET_DEVICE:
      Serial.println("Init: set device report format");
      device.set_device();
      init_phase = INIT_HEARTBEAT;
      break;
    case INIT_HEARTBEAT:
      Serial.println("Init: heartbeat (CONNECT)");
      device.transport.heartbeat();
      init_phase = INIT_WAKE;
      break;
    case INIT_WAKE:
      Serial.println("Init: wake screen");
      device.wakeScreen();
      init_phase = INIT_BRIGHTNESS;
      break;
    case INIT_BRIGHTNESS:
      Serial.println("Init: key brightness");
      device.set_brightness(100);
      init_phase = INIT_CLEAR;
      break;
    case INIT_CLEAR:
      Serial.println("Init: clear icons");
      device.clearAllIcon();
      init_phase = INIT_REFRESH;
      break;
    case INIT_REFRESH:
      Serial.println("Init: refresh display");
      device.refresh();
      init_phase = INIT_KB_BACKLIGHT;
      break;
    case INIT_KB_BACKLIGHT:
      device.set_keyboard_backlight_brightness(4);
      init_phase = INIT_KB_FX;
      break;
    case INIT_KB_FX:
      device.set_keyboard_lighting_effects(0);
      init_phase = INIT_KB_RGB;
      break;
    case INIT_KB_RGB:
      device.set_keyboard_rgb_backlight(0, 64, 128);
      init_phase = INIT_DONE;
      device.set_config_callback(onConfigEvent);
      device_ready = true;
      Serial.println("K1Pro ready. Press keys/knobs or type '?' for commands.");
      break;
    default:
      break;
  }

  for (uint8_t i = 0; i < 8; i++) {
    myusb.Task();
  }
}

void onK1ProDisconnected() {
  if (init_phase == INIT_NONE && !device_ready) {
    return;
  }

  device.close();
  device_ready = false;
  init_phase = INIT_NONE;
  Serial.println("K1Pro disconnected");
}

// ---------------------------------------------------------------------------
// Serial command handler
// ---------------------------------------------------------------------------

void printHelp() {
  Serial.println();
  Serial.println("K1Pro test commands:");
  Serial.println("  ?              help");
  Serial.println("  b <0-100>      key brightness");
  Serial.println("  r              refresh display");
  Serial.println("  c              clear all key icons");
  Serial.println("  h              send heartbeat");
  Serial.println("  kb <0-6>       keyboard backlight brightness");
  Serial.println("  fx <0-9>       keyboard lighting effect");
  Serial.println("  sp <0-7>       keyboard lighting speed");
  Serial.println("  rgb <r> <g> <b> keyboard RGB color");
  Serial.println("  os <0|1>       OS mode (0=Windows, 1=macOS)");
  Serial.println("  mode <0|1>     keyboard mode (0=native, 1=SDK)");
  Serial.println("  image <1-6> <file>  set key image from SD card");
  Serial.println();
}

void handleSerialCommand(const char *line) {
  if (!device_ready) {
    Serial.println("K1Pro not connected");
    return;
  }

  if (line[0] == '\0') {
    return;
  }

  if (strcmp(line, "?") == 0 || strcmp(line, "help") == 0) {
    printHelp();
    return;
  }

  if (strcmp(line, "r") == 0) {
    device.refresh();
    Serial.println("Display refreshed");
    return;
  }

  if (strcmp(line, "c") == 0) {
    device.clearAllIcon();
    device.refresh();
    Serial.println("All key icons cleared");
    return;
  }

  if (strcmp(line, "h") == 0) {
    device.transport.heartbeat();
    Serial.println("Heartbeat sent");
    return;
  }

  int value = 0;
  int r = 0, g = 0, b = 0;
  if (sscanf(line, "b %d", &value) == 1) {
    device.set_brightness(value);
    Serial.printf("Key brightness set to %d\n", value);
    return;
  }

  if (sscanf(line, "kb %d", &value) == 1) {
    device.set_keyboard_backlight_brightness(value);
    Serial.printf("Keyboard backlight set to %d\n", value);
    return;
  }

  if (sscanf(line, "fx %d", &value) == 1) {
    device.set_keyboard_lighting_effects(value);
    Serial.printf("Keyboard effect set to %d\n", value);
    return;
  }

  if (sscanf(line, "sp %d", &value) == 1) {
    device.set_keyboard_lighting_speed(value);
    Serial.printf("Keyboard speed set to %d\n", value);
    return;
  }

  if (sscanf(line, "rgb %d %d %d", &r, &g, &b) == 3) {
    device.set_keyboard_rgb_backlight(r, g, b);
    Serial.printf("Keyboard RGB set to (%d, %d, %d)\n", r, g, b);
    return;
  }

  if (sscanf(line, "os %d", &value) == 1) {
    device.keyboard_os_mode_switch(value);
    Serial.printf("OS mode set to %d\n", value);
    return;
  }

  if (sscanf(line, "mode %d", &value) == 1) {
    device.keyboard_mode(value);
    Serial.printf("Keyboard mode set to %d\n", value);
    return;
  }
  char filename[100];
  if (sscanf(line, "image %d %99s", &value, filename) == 2) {
    const int result = device.set_key_image(value, filename);
    if (result == 0) {
      Serial.printf("Key %d image set from %s\n", value, filename);
    } else {
      Serial.printf("Failed to set key %d image from %s\n", value, filename);
    }
    return;
  }

  Serial.printf("Unknown command: %s (type ? for help)\n", line);
}

void pollSerialCommands() {
  static char line[80];
  static uint8_t len = 0;

  while (Serial.available()) {
    const char c = Serial.read();
    if (c == '\r') {
      continue;
    }
    if (c == '\n') {
      line[len] = '\0';
      handleSerialCommand(line);
      len = 0;
      continue;
    }
    if (len < sizeof(line) - 1) {
      line[len++] = c;
    }
  }
}

// ---------------------------------------------------------------------------
// Arduino entry points
// ---------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    // wait for Serial Monitor
  }

  Serial.println();
  Serial.println("========================================");
  Serial.println("  K1Pro Teensy USB Host Test");
  Serial.println("========================================");

  MiraBoxHIDInput::show_raw_data = false;
  MiraBoxHIDInput::show_formated_data = false;

  if (!SD.begin(BUILTIN_SDCARD)) {
    Serial.println("SD card initialization failed!");
  }

  transport.configureK1ProDefaults();
  myusb.begin();

  printHelp();
  Serial.println("Waiting for K1Pro...");

  
}

void loop() {
  myusb.Task();

  for (uint8_t i = 0; i < CNT_HIDDEVICES; ++i) {
    if (*hiddrivers[i] != hid_driver_active[i]) {
      if (hid_driver_active[i]) {
        hid_driver_active[i] = false;
        if (hiddrivers[i] == &hdc2) {
          onK1ProDisconnected();
        }
      } else {
        hid_driver_active[i] = true;
        if (hiddrivers[i] == &hdc2) {
          onK1ProConnected();
        }
      }
    }
  }

  if (device_ready) {
    device.poll();
  }

  serviceDeferredInit();
  pollSerialCommands();
}
