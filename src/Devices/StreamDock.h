#pragma once

#include "../FeatrueOption.h"
#include "../ImageFormat.h"
#include "../InputTypes.h"
#include "../Transport/LibUSBHIDAPI.h"
#include "../DeviceConfigEvent.h"
#include "../DeviceConfig.h"
#include "GifController.h"

#include <cstddef>
#include <cstdint>
#include <string>

class StreamDock {
public:
    static constexpr int KEY_COUNT = 0;
    static constexpr int KEY_COLS = 0;
    static constexpr int KEY_ROWS = 0;
    static constexpr int KEY_PIXEL_WIDTH = 0;
    static constexpr int KEY_PIXEL_HEIGHT = 0;
    static constexpr int DIAL_COUNT = 0;
    static constexpr bool KEY_MAP = false;
    static constexpr bool DECK_VISUAL = false;
    static constexpr bool DECK_TOUCH = false;

    using KeyCallback = void (*)(StreamDock *, const InputEvent &);
    using ConfigCallback = void (*)(StreamDock *, const DeviceConfigEvent &);
    using RawReadCallback = void (*)(StreamDock *, const uint8_t *, size_t);
    using TouchscreenCallback = void (*)(StreamDock *, const InputEvent &);

    explicit StreamDock(LibUSBHIDAPI &transport, const HidDeviceInfo &dev_info);
    virtual ~StreamDock();

    virtual bool open();
    virtual void init();
    virtual void close(bool notify = true);
    virtual void set_device() = 0;

    void disconnected();
    void clearIcon(int index);
    void clearAllIcon();
    void wakeScreen();
    void refresh();

    void set_led_brightness(int percent);
    void set_led_color(int r, int g, int b);
    void reset_led_effect();
    int send_config();

    size_t read(uint8_t *buffer, size_t capacity);
    void poll();

    void set_key_callback(KeyCallback callback);
    void set_config_callback(ConfigCallback callback);
    void set_raw_read_callback(RawReadCallback callback);
    void set_touchscreen_callback(TouchscreenCallback callback);

    GifController &gifer() { return gif_controller_; }
    void set_key_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int key);
    void clear_key_gif(int key);
    void set_background_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int x = 0, int y = 0, uint8_t fb_layer = 0x00);
    void clear_background_gif(int position = 0x03);
    void start_gif_loop();
    void stop_gif_loop();
    bool gif_loop_status() const;

    std::string getPath() const;
    std::string get_serial_number() const;
    std::string id() const;

    virtual int set_brightness(int percent) = 0;
    virtual int set_key_image(int key, const char *filename);
    virtual int set_key_imageData(int key, const uint8_t *data, size_t length) = 0;
    virtual int set_touchscreen_image(const uint8_t *data, size_t length) = 0;
    virtual int get_image_key(ButtonKey logical_key) = 0;
    virtual InputEvent decode_input_event(int hardware_code, int state) = 0;
    virtual DeviceConfigEvent decode_device_config_event(const uint8_t *data, size_t length);
    virtual int key_count() const { return 0; }
    virtual ImageFormat key_image_format() const;
    virtual ImageFormat touchscreen_image_format() const;

    LibUSBHIDAPI &transport;
    FeatrueOption feature_option;
    DeviceConfig *config = nullptr;

    uint16_t vendor_id = 0;
    uint16_t product_id = 0;
    std::string path;
    std::string serial_number;
    std::string firmware_version;

protected:
    virtual void handle_raw_read(const uint8_t *data, size_t length);
    void dispatch_touchscreen_event(const InputEvent &event);
    bool is_input_event_packet(const uint8_t *data, size_t length) const;
    bool is_input_device_config_packet(const uint8_t *data, size_t length) const;

    GifController gif_controller_;

private:
    void process_read_buffer(const uint8_t *data, size_t length);
    void service_heartbeat();

    KeyCallback key_callback_ = nullptr;
    ConfigCallback config_callback_ = nullptr;
    RawReadCallback raw_read_callback_ = nullptr;
    TouchscreenCallback touchscreen_callback_ = nullptr;

    bool notify_on_close_ = true;
    bool run_poll_ = false;
    unsigned long last_heartbeat_ms_ = 0;

};
