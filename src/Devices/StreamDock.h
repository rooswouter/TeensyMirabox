#pragma once

#include "../FeatureOption.h"
#include "../ImageFormat.h"
#include "../InputTypes.h"
#include "../Transport/LibUSBHIDAPI.h"
#include "../DeviceConfigEvent.h"
#include "../KeyboardEvent.h"
#include "../DeviceConfig.h"
#include "GifController.h"
#include "GifLoader.h"

#include <cstddef>
#include <cstdint>
#include <string>

/**
 * @file StreamDock.h
 * @brief Base class for all StreamDock / Mirabox USB devices.
 *
 * Provides image upload, brightness, LED control, input callbacks, GIF animation,
 * and periodic polling. Subclasses implement device-specific report sizes, key
 * maps, and image formats.
 */

/**
 * @class StreamDock
 * @brief Abstract StreamDock device driver.
 */
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

    /** @brief Called when a button, knob, swipe, or DIP event occurs. */
    using KeyCallback = void (*)(StreamDock *, const InputEvent &);

    /** @brief Called when a K1 Pro configuration packet is received. */
    using ConfigCallback = void (*)(StreamDock *, const DeviceConfigEvent &);

    /** @brief Called when a K1 Pro keyboardpacket is received. */
    using KeyboardCallback = void (*)(StreamDock *, const KeyboardEvent &);

    /** @brief Called for every raw HID input report (before parsing). */
    using RawReadCallback = void (*)(StreamDock *, const uint8_t *, size_t);

    /** @brief Called for touchscreen / touch-bar events. */
    using TouchscreenCallback = void (*)(StreamDock *, const InputEvent &);

    /**
     * @brief Construct a device bound to a transport and USB identity.
     * @param transport_ref LibUSBHIDAPI for the control HID interface.
     * @param dev_info Vendor/product ID, serial, and path strings.
     */
    explicit StreamDock(LibUSBHIDAPI &transport_ref, const HidDeviceInfo &dev_info);

    virtual ~StreamDock();

    /**
     * @brief Open the HID transport and start the poll/heartbeat loop.
     * @return `true` on success.
     */
    virtual bool open();

    /**
     * @brief Standard device initialization sequence.
     *
     * Calls `set_device()`, wakes the screen, sets brightness, clears icons,
     * reads firmware version, and refreshes the display.
     */
    virtual void init();

    /**
     * @brief Close the device and release callbacks.
     * @param notify If true, send a disconnect command to the device.
     */
    virtual void close(bool notify = true);

    /**
     * @brief Configure report sizes, report ID, and feature flags for this model.
     *
     * Must be implemented by each subclass. Called by `init()` and DeviceManager.
     */
    virtual void set_device() = 0;

    /** @brief Send disconnect notification to firmware. */
    void disconnected();

    /**
     * @brief Clear one key image.
     * @param index Logical key index (1-based).
     */
    void clearIcon(int index);

    /** @brief Clear all key images. */
    void clearAllIcon();

    /** @brief Wake the LCD (`DIS`). */
    void wakeScreen();

    /**
     * @brief Push pending image data to the display (`STP`).
     *
     * Call after one or more `set_key_image` / `set_key_imageData` calls.
     */
    void refresh();

    /**
     * @brief Set RGB LED strip brightness (if supported).
     * @param percent Brightness 0–100.
     */
    void set_led_brightness(int percent);

    /**
     * @brief Set uniform LED color on supported devices.
     */
    void set_led_color(int r, int g, int b);

    /** @brief Reset LED effects to default. */
    void reset_led_effect();

    /**
     * @brief Send `config` object bytes to the device.
     * @return 0 on success, -1 if no config object is set.
     */
    int send_config();

    /**
     * @brief Read one HID input report from the transport.
     * @return Number of bytes read.
     */
    size_t read(uint8_t *buffer, size_t capacity);

    /**
     * @brief Process input reports, GIF frames, and heartbeat.
     *
     * Call frequently from `loop()` while the device is open.
     */
    void poll();

    /** @brief Register handler for button/knob/swipe events. */
    void set_key_callback(KeyCallback callback);

    /** @brief Register handler for K1 Pro configuration packets. */
    void set_config_callback(ConfigCallback callback);

    /** @brief Register handler for K1 Pro keyboard packets. */
    void set_keyboard_callback(KeyboardCallback callback);

    /** @brief Register handler for raw HID input bytes. */
    void set_raw_read_callback(RawReadCallback callback);

    /** @brief Register handler for touchscreen / touch-bar events. */
    void set_touchscreen_callback(TouchscreenCallback callback);

    /** @brief Access the GIF animation controller. */
    GifController &gifer() { return gif_controller_; }

    /**
     * @brief Assign an in-memory GIF to a key.
     * @param frames Array of pointers to JPEG frame data.
     * @param frame_sizes Byte length of each frame.
     * @param delays Display time per frame in milliseconds.
     * @param frame_count Number of frames.
     * @param key Logical key index (1-based).
     */
    void set_key_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int key);

    /** @brief Stop GIF playback on a key. */
    void clear_key_gif(int key);

    /** @brief Assign an in-memory background GIF. */
    void set_background_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int x = 0, int y = 0, uint8_t fb_layer = 0x00);

    /** @brief Stop background GIF on a layer. */
    void clear_background_gif(int position = 0x03);

    /** @brief Start the GIF update loop (call from `poll()` via `gifer().update()`). */
    void start_gif_loop();

    /** @brief Stop the GIF update loop. */
    void stop_gif_loop();

    /** @return True if the GIF loop is running. */
    bool gif_loop_status() const;

    /**
     * @brief Load a GIF from the SD card and animate it on a key.
     *
     * Requires `ENABLE_ANIMATEDGIF` and the AnimatedGIF + JPEGENC libraries.
     * @return 0 on success, -1 on failure or when GIF support is disabled.
     */
    int set_key_gif(int key, const char *filename);

    /**
     * @brief Load a GIF from memory and animate it on a key.
     * @return 0 on success, -1 on failure or when GIF support is disabled.
     */
    int set_key_gif_data(int key, const uint8_t *data, size_t length);

    /**
     * @brief Load a background GIF from the SD card.
     * @return 0 on success, -1 on failure or when GIF support is disabled.
     */
    int set_background_gif(const char *filename, int x = 0, int y = 0, uint8_t fb_layer = 0x00);

    /**
     * @brief Load a background GIF from memory.
     * @return 0 on success, -1 on failure or when GIF support is disabled.
     */
    int set_background_gif_data(const uint8_t *data, size_t length, int x = 0, int y = 0, uint8_t fb_layer = 0x00);

    /** @return Device path or serial string. */
    std::string getPath() const;

    /** @return USB serial number string. */
    std::string get_serial_number() const;

    /** @return Unique device identifier (same as path). */
    std::string id() const;

    /** @return Device type enum for this instance. */
    device_type get_device_type() const { return feature_option.deviceType; }

    /**
     * @brief Set LCD brightness (device-specific implementation).
     * @param percent Brightness 0–100.
     * @return 0 on success, negative on error.
     */
    virtual int set_brightness(int percent) = 0;

    /**
     * @brief Load a JPEG from the SD card and assign it to a key.
     * @param key Logical key index (1-based).
     * @param filename Path on the SD card (e.g. `"key1.jpg"`).
     * @return 0 on success, -1 on failure.
     */
    virtual int set_key_image(int key, const char *filename);

    /**
     * @brief Upload raw JPEG bytes to a key.
     * @param key Logical key index (1-based).
     * @param data JPEG data.
     * @param length Data length.
     * @return 0 on success, negative on error.
     */
    virtual int set_key_imageData(int key, const uint8_t *data, size_t length) = 0;

    /**
     * @brief Set the touchscreen background image.
     * @return 0 on success, negative on error.
     */
    virtual int set_touchscreen_image(const uint8_t *data, size_t length) = 0;

    /**
     * @brief Map a logical key to a hardware key index for image upload.
     * @param logical_key Application key (ButtonKey enum).
     * @return Hardware key index, or -1 if unsupported.
     */
    virtual int get_image_key(ButtonKey logical_key) = 0;

    /**
     * @brief Decode a hardware button/knob code into an InputEvent.
     * @param hardware_code Raw byte from the HID report.
     * @param state Press/release state from the report.
     */
    virtual InputEvent decode_input_event(int hardware_code, int state) = 0;

    /**
     * @brief Parse a K1 Pro configuration HID packet.
     */
    virtual DeviceConfigEvent decode_device_config_event(const uint8_t *data, size_t length);

    /**
     * @brief Number of keys that accept images.
     * @return Key count for this device model.
     */
    virtual int image_keys() const { return 0; }

    /** @return Required key image dimensions, format, and rotation. */
    virtual ImageFormat key_image_format() const;

    /** @return Required touchscreen background format. */
    virtual ImageFormat touchscreen_image_format() const;

    LibUSBHIDAPI &transport;
    FeatureOption feature_option;
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
    bool is_input_keyboard_packet(const uint8_t *data, size_t length) const;

    

    GifController gif_controller_;
    GifLoader gif_loader_;

private:
    bool resolve_sd_path(const char *filename, const char *subdir, char *sd_path, size_t sd_path_len) const;
    void process_read_buffer(const uint8_t *data, size_t length);
    void service_heartbeat();

    KeyCallback key_callback_ = nullptr;
    ConfigCallback config_callback_ = nullptr;
    KeyboardCallback keyboard_callback_ = nullptr;
    RawReadCallback raw_read_callback_ = nullptr;
    TouchscreenCallback touchscreen_callback_ = nullptr;

    bool notify_on_close_ = true;
    bool run_poll_ = false;
    unsigned long last_heartbeat_ms_ = 0;

};

