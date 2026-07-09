#pragma once

#include <Arduino.h>
#include <cstdint>
#include <cstring>
#include <string>

class MiraBoxHIDInput;

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

class LibUSBHIDAPI {
public:
    static constexpr size_t DEFAULT_REPORT_SIZE = 1024;

    explicit LibUSBHIDAPI(MiraBoxHIDInput &hid);

    bool open();
    void close();
    bool canWrite() const;

    std::string getFirmwareVersion();
    void clearTaskQueue() {}

    size_t read(uint8_t *buffer, size_t capacity, int timeout_ms = -1);
    size_t read_(size_t size);

    void wakeupScreen();
    void magneticCalibration();
    void refreshScreen();
    void sleep();

    void setKeyBrightness(int brightness);
    void clearAllKeys();
    void clearKey(int key_index);

    void setKeyImageStream(const uint8_t *jpeg_data, size_t jpeg_len, int key_index);
    void setBackgroundBitmap(const uint8_t *bitmap_data, size_t bitmap_len, uint32_t timeout_ms = 5000);
    void setBackgroundImageStream(const uint8_t *jpeg_data, size_t jpeg_len, uint32_t timeout_ms = 3000);
    void setBackgroundFrameStream(
        const uint8_t *jpeg_data,
        size_t jpeg_len,
        int width,
        int height,
        int x = 0,
        int y = 0,
        uint8_t fb_layer = 0x00
    );
    void clearBackgroundFrameStream(int position = 0x03);

    void setLedBrightness(int brightness);
    void setLedColor(int count, int r, int g, int b);
    void resetLedColor();
    void setLedColorIndividual(
        uint8_t ar, uint8_t ag, uint8_t ab,
        uint8_t br, uint8_t bg, uint8_t bb,
        uint8_t cr, uint8_t cg, uint8_t cb,
        uint8_t dr, uint8_t dg, uint8_t db
    );

    void setKeyboardBacklightBrightness(int brightness);
    void setKeyboardLightingEffects(int effect);
    void setKeyboardLightingSpeed(int speed);
    void setKeyboardRgbBacklight(int red, int green, int blue);
    void keyboardOsModeSwitch(int os_mode);

    void setDeviceConfig(const uint8_t *configs, size_t config_len);
    void changeMode(int mode);
    void changePage(int page);
    void setN1SkinBitmap(
        const uint8_t *png_data,
        size_t png_len,
        int skin_mode,
        int skin_page,
        int skin_status,
        int key_index,
        uint32_t timeout_ms = 3000
    );

    void heartbeat();
    void notifyDisconnected();

    void setReportId(uint8_t report_id);
    uint8_t getReportId() const;
    void setReportSize(uint16_t input_report_size, uint16_t output_report_size, uint16_t feature_report_size);

    uint16_t inputReportSize() const { return input_report_size_; }
    uint16_t outputReportSize() const { return output_report_size_; }
    uint16_t featureReportSize() const { return feature_report_size_; }

    std::string getLastError() const;

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
