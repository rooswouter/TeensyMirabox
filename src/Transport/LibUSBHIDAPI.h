#pragma once

#include <Arduino.h>
#include <cstdint>
#include <cstring>
#include <string>

class MiraBoxHIDInput;

/**
 * @brief USB identification and strings for a connected HID device.
 */
struct HidDeviceInfo {
    std::string path;
    uint16_t vendor_id = 0;
    uint16_t product_id = 0;
    std::string serial_number;
    uint16_t release_number = 0;
    std::string manufacturer_string;
    std::string product_string;
    uint16_t usage_page = 0;
    uint16_t usage = 0;
    int interface_number = 0;
};

/**
 * @file LibUSBHIDAPI.h
 * @brief StreamDock HID transport (CRT command protocol).
 *
 * Sends and receives HID reports through a `MiraBoxHIDInput` control interface.
 * API mirrors the Python `LibUSBHIDAPI` transport layer.
 */

/**
 * @class LibUSBHIDAPI
 * @brief Low-level HID transport for StreamDock firmware commands.
 */
class LibUSBHIDAPI {
public:
    static constexpr size_t DEFAULT_REPORT_SIZE = 1024;

    /**
     * @brief Bind transport to a control HID input driver.
     * @param hid MiraBoxHIDInput instance with `index` 2 (control).
     */
    explicit LibUSBHIDAPI(MiraBoxHIDInput &hid);

    /**
     * @brief Mark transport open if the HID interface is connected.
     * @return `true` if the control interface is available.
     */
    bool open();

    /** @brief Mark transport closed (does not disconnect USB). */
    void close();

    /**
     * @brief True if the device is open and the HID link is up.
     */
    bool canWrite() const;

    /**
     * @brief Request firmware version via HID GET_REPORT (Input).
     * @return Firmware string, or empty if unavailable.
     */
    std::string getFirmwareVersion();

    /** @brief No-op on Teensy (kept for Python API compatibility). */
    void clearTaskQueue() {}

    /**
     * @brief Read one input report from the HID queue.
     * @param buffer Destination buffer.
     * @param capacity Buffer size.
     * @param timeout_ms Wait time in ms; 0 returns immediately.
     * @return Bytes read, or 0 if no data.
     */
    size_t read(uint8_t *buffer, size_t capacity, int timeout_ms = -1);

    /**
     * @brief Read with a fixed-size buffer and 100 ms timeout (legacy helper).
     * @param size Ignored; kept for API compatibility.
     */
    size_t read_(size_t size);

    /** @brief Send CRT `DIS` — wake / display on. */
    void wakeupScreen();

    /** @brief Send CRT `CHECK` — magnetic calibration (supported devices). */
    void magneticCalibration();

    /** @brief Send CRT `STP` — refresh the display after image updates. */
    void refreshScreen();

    /** @brief Send CRT `HAN` — sleep the device screen. */
    void sleep();

    /**
     * @brief Set key LCD brightness.
     * @param brightness Brightness value (typically 0–100).
     */
    void setKeyBrightness(int brightness);

    /** @brief Clear every key icon (`CLE` with index 0xFF). */
    void clearAllKeys();

    /**
     * @brief Clear one key icon.
     * @param key_index Hardware key index (0xFF = all keys).
     */
    void clearKey(int key_index);

    /**
     * @brief Upload a JPEG image to a key (`BAT` command + bulk data).
     * @param jpeg_data JPEG file bytes.
     * @param jpeg_len Length of JPEG data.
     * @param key_index Hardware key index.
     */
    void setKeyImageStream(const uint8_t *jpeg_data, size_t jpeg_len, int key_index);

    /**
     * @brief Set full-screen background from raw bitmap data.
     * @param bitmap_data Raw pixel data.
     * @param bitmap_len Data length.
     * @param timeout_ms Unused on Teensy (kept for API compatibility).
     */
    void setBackgroundBitmap(const uint8_t *bitmap_data, size_t bitmap_len, uint32_t timeout_ms = 5000);

    /**
     * @brief Set full-screen background from JPEG data.
     */
    void setBackgroundImageStream(const uint8_t *jpeg_data, size_t jpeg_len, uint32_t timeout_ms = 3000);

    /**
     * @brief Draw a JPEG frame at a position (`BGPIC` + bulk).
     */
    void setBackgroundFrameStream(
        const uint8_t *jpeg_data,
        size_t jpeg_len,
        int width,
        int height,
        int x = 0,
        int y = 0,
        uint8_t fb_layer = 0x00
    );

    /**
     * @brief Clear a background framebuffer layer (`BGCLE`).
     * @param position Layer index (default 0x03).
     */
    void clearBackgroundFrameStream(int position = 0x03);

    /** @brief Set RGB LED strip brightness (`LBLIG`). */
    void setLedBrightness(int brightness);

    /**
     * @brief Set the same RGB color on the first N LEDs (`LCRGB`).
     * @param count Number of LEDs (max 4).
     */
    void setLedColor(int count, int r, int g, int b);

    /** @brief Reset LED color effects. */
    void resetLedColor();

    /** @brief Set four individual LED colors (N4 Pro style). */
    void setLedColorIndividual(
        uint8_t ar, uint8_t ag, uint8_t ab,
        uint8_t br, uint8_t bg, uint8_t bb,
        uint8_t cr, uint8_t cg, uint8_t cb,
        uint8_t dr, uint8_t dg, uint8_t db
    );

    /** @brief K1 Pro keyboard backlight brightness. */
    void setKeyboardBacklightBrightness(int brightness);

    /** @brief K1 Pro keyboard lighting effect index. */
    void setKeyboardLightingEffects(int effect);

    /** @brief K1 Pro keyboard lighting animation speed. */
    void setKeyboardLightingSpeed(int speed);

    /** @brief K1 Pro keyboard RGB backlight color. */
    void setKeyboardRgbBacklight(int red, int green, int blue);

    /**
     * @brief K1 Pro OS mode switch.
     * @param os_mode 0 = Windows, 1 = macOS.
     */
    void keyboardOsModeSwitch(int os_mode);

    /**
     * @brief Send device configuration blob (`DEVCFG`).
     */
    void setDeviceConfig(const uint8_t *configs, size_t config_len);

    /** @brief N1 mode change command. */
    void changeMode(int mode);

    /** @brief N1 page change command. */
    void changePage(int page);

    /**
     * @brief Upload N1 skin bitmap (PNG).
     */
    void setN1SkinBitmap(
        const uint8_t *png_data,
        size_t png_len,
        int skin_mode,
        int skin_page,
        int skin_status,
        int key_index,
        uint32_t timeout_ms = 3000
    );

    /** @brief Send CRT `CONNECT` keep-alive heartbeat. */
    void heartbeat();

    /** @brief Notify device of host disconnect (`CLE..DC`). */
    void notifyDisconnected();

    /**
     * @brief Set HID report ID prepended to output packets.
     * @param report_id 0x00 for most docks, 0x04 for K1 Pro.
     */
    void setReportId(uint8_t report_id);

    /** @return Current output report ID. */
    uint8_t getReportId() const;

    /**
     * @brief Configure logical HID report sizes.
     * @param input_report_size Input report length (including report ID byte).
     * @param output_report_size Output report length.
     * @param feature_report_size Feature report length (often 0).
     */
    void setReportSize(uint16_t input_report_size, uint16_t output_report_size, uint16_t feature_report_size);

    /** @return Configured input report size. */
    uint16_t inputReportSize() const { return input_report_size_; }

    /** @return Configured output report size. */
    uint16_t outputReportSize() const { return output_report_size_; }

    /** @return Configured feature report size. */
    uint16_t featureReportSize() const { return feature_report_size_; }

    /** @return Last write error message, or empty string. */
    std::string getLastError() const;

    /**
     * @brief Pre-configure report sizes and report ID for K1 Pro.
     *
     * Call before `open()` when binding manually (see `k1pro.ino`).
     */
    void configureK1ProDefaults();

private:
    size_t reportPayloadSize() const;
    size_t paddedReportSize() const;
    size_t padPacket(uint8_t *dest, size_t dest_capacity, const uint8_t *src, size_t src_len) const;
    bool writePacket(const uint8_t *data, size_t length, unsigned long timeout_ms = 10000);
    void sendCrt(const char *cmd, const uint8_t *params = nullptr, size_t params_len = 0, const uint8_t *bulk = nullptr, size_t bulk_len = 0, const uint8_t *custom_crt = nullptr, size_t custom_crt_len = 0, bool no_crt_header = false);

    MiraBoxHIDInput &hid_;
    bool is_open_ = false;
    // Matches the Python transport default; K1Pro overrides to 0x04 in set_device().
    uint8_t report_id_ = 0x00;
    uint16_t input_report_size_ = 513;
    uint16_t output_report_size_ = 1024;
    uint16_t feature_report_size_ = 0;
    std::string last_error_;
};
