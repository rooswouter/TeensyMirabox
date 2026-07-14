#include "StreamDock.h"

#include <Arduino.h>
#include "../DeviceConfig.h"
#include <cstdio>
#include <vector>

#include <SD.h>
#include <SPI.h>

const char* const device_type_names[] = {
    "StreamDockUniversal",
    "StreamDock293",
    "StreamDock293V3",
    "StreamDock293s",
    "StreamDock293sV3",
    "StreamDockM3",
    "StreamDockM18",
    "StreamDockN1",
    "StreamDockN3",
    "StreamDockN4",
    "StreamDockN4Pro",
    "StreamDockXL",
    "K1Pro",
    "StreamDockMini",
};

StreamDock::StreamDock(LibUSBHIDAPI &transport_ref, const HidDeviceInfo &dev_info)
    : transport(transport_ref), gif_controller_(*this) {
    vendor_id = dev_info.vendor_id;
    product_id = dev_info.product_id;
    path = dev_info.path;
    serial_number = dev_info.serial_number;

    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("SD card initialization failed!");
    }
}

StreamDock::~StreamDock() {
    delete config;
    config = nullptr;
    close(false);
}

bool StreamDock::open() {
    if (!transport.open()) {
        return false;
    }
    notify_on_close_ = true;
    run_poll_ = true;
    last_heartbeat_ms_ = millis();
    return true;
}

void StreamDock::init() {
    set_device();
    wakeScreen();
    set_brightness(100);
    clearAllIcon();
    firmware_version = transport.getFirmwareVersion();
    refresh();
}

void StreamDock::close(bool notify) {
    if (!notify) {
        notify_on_close_ = false;
    }

    gif_controller_.close();
    run_poll_ = false;

    if (notify && notify_on_close_) {
        disconnected();
    }

    transport.close();
    key_callback_ = nullptr;
    config_callback_ = nullptr;
    keyboard_callback_ = nullptr;
    raw_read_callback_ = nullptr;
    touchscreen_callback_ = nullptr;
}

void StreamDock::disconnected() {
    transport.notifyDisconnected();
}

void StreamDock::clearIcon(int index) {
    if (index < 1 || index > image_keys()) {
        return;
    }
  const int hardware_key = get_image_key(static_cast<ButtonKey>(index));
    if (hardware_key >= 0) {
        transport.clearKey(hardware_key);
    }
}

void StreamDock::clearAllIcon() {
    transport.clearAllKeys();
}

void StreamDock::wakeScreen() {
    transport.wakeupScreen();
}

void StreamDock::refresh() {
    transport.refreshScreen();
}

void StreamDock::set_led_brightness(int percent) {
    if (feature_option.hasRGBLed) {
        transport.setLedBrightness(percent);
    }
}

void StreamDock::set_led_color(int r, int g, int b) {
    if (feature_option.hasRGBLed) {
        transport.setLedColor(feature_option.ledCounts, r, g, b);
    }
}

void StreamDock::reset_led_effect() {
    if (feature_option.hasRGBLed) {
        transport.resetLedColor();
    }
}

int StreamDock::send_config() {
    if (config == nullptr) {
        return -1;
    }
    const std::vector<uint8_t> bytes = config->to_bytes();
    transport.setDeviceConfig(bytes.data(), bytes.size());
    return 0;
}

size_t StreamDock::read(uint8_t *buffer, size_t capacity) {
    return transport.read(buffer, capacity, 0);
}

void StreamDock::poll() {
    if (!run_poll_) {
        return;
    }

    static uint8_t buffer[1024];
    const size_t length = read(buffer, sizeof(buffer));
       if (length > 0) {
        process_read_buffer(buffer, length);
    }

    gif_controller_.update();
    service_heartbeat();
}

void StreamDock::set_key_callback(KeyCallback callback) {
    key_callback_ = callback;
}

void StreamDock::set_config_callback(ConfigCallback callback) {
    config_callback_ = callback;
}

void StreamDock::set_keyboard_callback(KeyboardCallback callback) {
    keyboard_callback_ = callback;
}

void StreamDock::set_raw_read_callback(RawReadCallback callback) {
    raw_read_callback_ = callback;
}

void StreamDock::set_touchscreen_callback(TouchscreenCallback callback) {
    touchscreen_callback_ = callback;
}

void StreamDock::set_key_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int key) {
    gif_controller_.set_key_gif_stream(frames, frame_sizes, delays, frame_count, key);
}

void StreamDock::clear_key_gif(int key) {
    gif_controller_.clear_key_gif(key);
}

void StreamDock::set_background_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int x, int y, uint8_t fb_layer) {
    gif_controller_.set_background_gif_stream(frames, frame_sizes, delays, frame_count, x, y, fb_layer);
}

void StreamDock::clear_background_gif(int position) {
    gif_controller_.clear_background_gif(position);
}

void StreamDock::start_gif_loop() {
    gif_controller_.start_gif_loop();
}

void StreamDock::stop_gif_loop() {
    gif_controller_.stop_gif_loop();
}

bool StreamDock::gif_loop_status() const {
    return gif_controller_.gif_loop_status();
}

std::string StreamDock::getPath() const {
    return path;
}

std::string StreamDock::get_serial_number() const {
    return serial_number;
}

std::string StreamDock::id() const {
    return getPath();
}

DeviceConfigEvent StreamDock::decode_device_config_event(const uint8_t *data, size_t length) {
    return DeviceConfigEvent(data, length);
}

ImageFormat StreamDock::key_image_format() const {
    return ImageFormat{};
}

ImageFormat StreamDock::touchscreen_image_format() const {
    return ImageFormat{};
}

void StreamDock::handle_raw_read(const uint8_t *data, size_t length) {
    if (raw_read_callback_ != nullptr) {
        raw_read_callback_(this, data, length);
    }
}

void StreamDock::dispatch_touchscreen_event(const InputEvent &event) {
    if (touchscreen_callback_ != nullptr) {
        touchscreen_callback_(this, event);
    }
}

bool StreamDock::is_input_event_packet(const uint8_t *data, size_t length) const {
   
    if (feature_option.deviceType == device_type::k1pro) {
        return length >= 11
            && data[0] == 0x04
            && data[1] == 0x41
            && data[2] == 0x43
            && data[3] == 0x4B
            && data[6] == 0x4F
            && data[7] == 0x4B;
    }

    return length >= 11
        && data[0] == 0x41
        && data[1] == 0x43
        && data[2] == 0x4B
        && data[5] == 0x4F
        && data[6] == 0x4B;
}

bool StreamDock::is_input_device_config_packet(const uint8_t *data, size_t length) const {
    if (feature_option.deviceType != device_type::k1pro) {
        return false;
    }

    return length >= 12
        && data[0] == 0x04
        && data[1] == 0x44
        && data[2] == 0x45
        && data[3] == 0x56
        && data[4] == 0x43
        && data[5] == 0x46
        && data[6] == 0x47;
}
bool StreamDock::is_input_keyboard_packet(const uint8_t *data, size_t length) const {
    if (feature_option.deviceType != device_type::k1pro) {
        return false;
    }
    return length == 8;
}

void StreamDock::process_read_buffer(const uint8_t *data, size_t length) {
    handle_raw_read(data, length);
    if (length < 8) {
        return;
    }
    
    if (length >= 10 && data[9] == 0xFF) {
        return;
    }
    
    if (is_input_event_packet(data, length)) {
        InputEvent event;
        if (feature_option.deviceType == device_type::k1pro) {
            event = decode_input_event(data[10], data[11]);
        } else {
            event = decode_input_event(data[9], data[10]);
        }
        if (event.event_type != EventType::UNKNOWN && key_callback_ != nullptr) {
            key_callback_(this, event);
        }
        return;
    } else if (is_input_device_config_packet(data, length)) {
        const DeviceConfigEvent event = decode_device_config_event(data, length);
        if (event.valid && config_callback_ != nullptr) {
            config_callback_(this, event);
        }
    } else if (is_input_keyboard_packet(data, length)) {
        if (keyboard_callback_ != nullptr) {
            keyboard_callback_(this, KeyboardEvent(data, length));
        }
    } 
}

void StreamDock::service_heartbeat() {
    const unsigned long now = millis();
    if (now - last_heartbeat_ms_ >= 10000UL) {
        transport.heartbeat();
        last_heartbeat_ms_ = now;
    }
}

int StreamDock::set_key_image(int key, const char *filename)
{
    if (filename == nullptr || filename[0] == '\0') {
        return -1;
    }

    // Open file from SD card, 50kb max?
    uint8_t buffer[50 * 1024];

    char sd_path[128];
    if (filename[0] == '/') {
        std::snprintf(sd_path, sizeof(sd_path), "%s", filename);
    } else {
        const int type_index = static_cast<int>(feature_option.deviceType);
        const char *folder = device_type_names[0];
        if (type_index >= 0 && type_index < static_cast<int>(sizeof(device_type_names) / sizeof(device_type_names[0]))) {
            folder = device_type_names[type_index];
        }
        std::snprintf(sd_path, sizeof(sd_path), "%s/key/%s", folder, filename);
    }

    File file = SD.open(sd_path, FILE_READ);
    if (!file) {
        printf("file %s not found\n", sd_path);
        return -1;
    }
    size_t read_size = file.read(buffer, sizeof(buffer));
    file.close();

    return set_key_imageData(key, buffer, read_size);
}
