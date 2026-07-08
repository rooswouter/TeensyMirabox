#include "LibUSBHIDAPI.h"
#include "../MiraBoxHIDInput.h"

namespace {

void writeUint16BE(uint8_t *buffer, uint16_t value) {
    buffer[0] = (uint8_t)(value >> 8);
    buffer[1] = (uint8_t)(value & 0xFF);
}

void writeUint32BE(uint8_t *buffer, uint32_t value) {
    buffer[0] = (uint8_t)(value >> 24);
    buffer[1] = (uint8_t)((value >> 16) & 0xFF);
    buffer[2] = (uint8_t)((value >> 8) & 0xFF);
    buffer[3] = (uint8_t)(value & 0xFF);
}

size_t trimTrailingNulls(const uint8_t *data, size_t length) {
    while (length > 0 && data[length - 1] == 0) {
        --length;
    }
    return length;
}

uint8_t s_tx_buffer[LibUSBHIDAPI::DEFAULT_REPORT_SIZE + 1];

} // namespace

LibUSBHIDAPI::LibUSBHIDAPI(MiraBoxHIDInput &hid) : hid_(hid) {}

bool LibUSBHIDAPI::open() {
    if (!hid_.isConnected()) {
        last_error_ = "Device not connected";
        return false;
    }
    is_open_ = true;
    last_error_.clear();
    return true;
}

void LibUSBHIDAPI::close() {
    is_open_ = false;
}

bool LibUSBHIDAPI::canWrite() const {
    return is_open_ && hid_.isConnected();
}

std::string LibUSBHIDAPI::getFirmwareVersion() {
    return "";
}

size_t LibUSBHIDAPI::read(uint8_t *buffer, size_t capacity, int timeout_ms) {
    (void)timeout_ms;

    if (!is_open_ || buffer == nullptr || capacity == 0) {
        return 0;
    }

    const unsigned long deadline = millis() + (timeout_ms < 0 ? 0UL : (unsigned long)timeout_ms);
    do {
        const size_t bytes_read = hid_.read(buffer, capacity);
        if (bytes_read > 0) {
            return trimTrailingNulls(buffer, bytes_read);
        }
        if (timeout_ms <= 0) {
            break;
        }
    } while ((int)(deadline - millis()) > 0);

    return 0;
}

size_t LibUSBHIDAPI::read_(size_t size) {
    (void)size;
    uint8_t buffer[DEFAULT_REPORT_SIZE + 1];
    return read(buffer, sizeof(buffer), 100);
}

size_t LibUSBHIDAPI::reportPayloadSize() const {
    if (output_report_size_ > 0) {
        return output_report_size_ - 1;
    }
    return DEFAULT_REPORT_SIZE;
}

size_t LibUSBHIDAPI::paddedReportSize() const {
    const uint16_t driver_out = hid_.outputReportSize();
    if (driver_out > 0) {
        return driver_out;
    }
    if (output_report_size_ > 0) {
        return output_report_size_;
    }
    return DEFAULT_REPORT_SIZE + 1;
}

size_t LibUSBHIDAPI::padPacket(uint8_t *dest, size_t dest_capacity, const uint8_t *src, size_t src_len) const {
    const size_t target = min(dest_capacity, paddedReportSize());
    const size_t copy_len = min(src_len, target);
    if (copy_len > 0) {
        memcpy(dest, src, copy_len);
    }
    if (copy_len < target) {
        memset(dest + copy_len, 0, target - copy_len);
    }
    return target;
}

bool LibUSBHIDAPI::writePacket(const uint8_t *data, size_t length, unsigned long timeout_ms) {
    if (!canWrite()) {
        last_error_ = "Device not open";
        return false;
    }

    size_t packet_len;
    if (data == s_tx_buffer) {
        const size_t target = min(sizeof(s_tx_buffer), paddedReportSize());
        if (length < target) {
            memset(s_tx_buffer + length, 0, target - length);
        }
        packet_len = target;
    } else {
        packet_len = padPacket(s_tx_buffer, sizeof(s_tx_buffer), data, length);
    }

    if (!hid_.sendPacket(s_tx_buffer, (int)packet_len, timeout_ms)) {
        last_error_ = "HID write failed";
        return false;
    }
    last_error_.clear();
    return true;
}


void LibUSBHIDAPI::sendCrt(const char *cmd, const uint8_t *params, size_t params_len, const uint8_t *bulk, size_t bulk_len, const uint8_t *custom_crt, size_t custom_crt_len,bool no_crt_header)
{
    if (!canWrite() || cmd == nullptr) {
        return;
    }

    size_t offset = 0;

    if (!no_crt_header) {
        if (custom_crt != nullptr) {
            const size_t crt_len = min(custom_crt_len, sizeof(s_tx_buffer));
            memcpy(s_tx_buffer, custom_crt, crt_len);
            offset = crt_len;
        } else {
            s_tx_buffer[0] = report_id_;
            s_tx_buffer[1] = 'C';
            s_tx_buffer[2] = 'R';
            s_tx_buffer[3] = 'T';
            s_tx_buffer[4] = 0;
            s_tx_buffer[5] = 0;
            offset = 6;
        }
    }
    const size_t cmd_len = strlen(cmd);
    if (offset + cmd_len > sizeof(s_tx_buffer)) {
        last_error_ = "CRT packet too large";
        return;
    }
    memcpy(s_tx_buffer + offset, cmd, cmd_len);
    offset += cmd_len;
    
    if (params != nullptr && params_len > 0) {
        if (offset + params_len > sizeof(s_tx_buffer)) {
            last_error_ = "CRT packet too large";
            return;
        }
        memcpy(s_tx_buffer + offset, params, params_len);
        offset += params_len;
    }
    if (!writePacket(s_tx_buffer, offset)) {
        return;
    }

    if (bulk == nullptr || bulk_len == 0) {
        return;
    }

    const size_t chunk_size = reportPayloadSize();
    constexpr unsigned long kBulkWriteTimeoutMs = 500;

    for (size_t i = 0; i < bulk_len; i += chunk_size) {
        const size_t chunk_len = min(chunk_size, bulk_len - i);
        s_tx_buffer[0] = report_id_;
        memcpy(s_tx_buffer + 1, bulk + i, chunk_len);
        if (!writePacket(s_tx_buffer, 1 + chunk_len, kBulkWriteTimeoutMs)) {
            return;
        }
        hid_.pumpHost();
    }
}

void LibUSBHIDAPI::wakeupScreen() {
    sendCrt("DIS");
}

void LibUSBHIDAPI::magneticCalibration() {
    sendCrt("CHECK");
}

void LibUSBHIDAPI::refreshScreen() {
    sendCrt("STP");
}

void LibUSBHIDAPI::sleep() {
    sendCrt("HAN");
}

void LibUSBHIDAPI::setKeyBrightness(int brightness) {
    uint8_t params[3];
    writeUint16BE(params, 0);
    params[2] = (uint8_t)brightness;
    sendCrt("LIG", params, sizeof(params));
}

void LibUSBHIDAPI::clearAllKeys() {
    clearKey(0xFF);
}

void LibUSBHIDAPI::clearKey(int key_index) {
    uint8_t params[4];
    writeUint16BE(params, 0);
    params[2] = 0;
    params[3] = (uint8_t)key_index;
    sendCrt("CLE", params, sizeof(params));
}

void LibUSBHIDAPI::setKeyImageStream(const uint8_t *jpeg_data, size_t jpeg_len, int key_index) {
    uint8_t params[5];
    writeUint32BE(params, (uint32_t)jpeg_len);
    params[4] = (uint8_t)key_index;
    sendCrt("BAT", params, sizeof(params), jpeg_data, jpeg_len);
}

void LibUSBHIDAPI::setBackgroundBitmap(const uint8_t *bitmap_data, size_t bitmap_len, uint32_t timeout_ms) {
    (void)timeout_ms;
    setKeyImageStream(bitmap_data, bitmap_len, 1);
}

void LibUSBHIDAPI::setBackgroundImageStream(const uint8_t *jpeg_data, size_t jpeg_len, uint32_t timeout_ms) {
    (void)timeout_ms;
    setKeyImageStream(jpeg_data, jpeg_len, 1);
}

void LibUSBHIDAPI::setBackgroundFrameStream(
    const uint8_t *jpeg_data,
    size_t jpeg_len,
    int width,
    int height,
    int x,
    int y,
    uint8_t fb_layer
) {
    uint8_t params[13];
    writeUint32BE(params, (uint32_t)jpeg_len);
    writeUint16BE(params + 4, (uint16_t)x);
    writeUint16BE(params + 6, (uint16_t)y);
    writeUint16BE(params + 8, (uint16_t)width);
    writeUint16BE(params + 10, (uint16_t)height);
    params[12] = fb_layer;
    sendCrt("BGPIC", params, sizeof(params), jpeg_data, jpeg_len);
}

void LibUSBHIDAPI::clearBackgroundFrameStream(int position) {
    uint8_t params[3];
    writeUint16BE(params, 0);
    params[2] = (uint8_t)position;
    sendCrt("BGCLE", params, sizeof(params));
}

void LibUSBHIDAPI::setLedBrightness(int brightness) {
    uint8_t params[1] = {(uint8_t)brightness};
    sendCrt("LBLIG", params, sizeof(params));
}

void LibUSBHIDAPI::setLedColor(int count, int r, int g, int b) {
    if (count < 0) {
        count = 0;
    }
    if (count > 4) {
        count = 4;
    }

    uint8_t params[12] = {0};
    for (int i = 0; i < count; ++i) {
        params[i * 3] = (uint8_t)r;
        params[i * 3 + 1] = (uint8_t)g;
        params[i * 3 + 2] = (uint8_t)b;
    }
    sendCrt("SETLB", params, sizeof(params));
}

void LibUSBHIDAPI::resetLedColor() {
    sendCrt("DELED");
}

void LibUSBHIDAPI::setLedColorIndividual(
    uint8_t ar, uint8_t ag, uint8_t ab,
    uint8_t br, uint8_t bg, uint8_t bb,
    uint8_t cr, uint8_t cg, uint8_t cb,
    uint8_t dr, uint8_t dg, uint8_t db
) {
    const uint8_t params[12] = {
        ar, ag, ab,
        br, bg, bb,
        cr, cg, cb,
        dr, dg, db,
    };
    sendCrt("SETLB", params, sizeof(params));
}

void LibUSBHIDAPI::setKeyboardBacklightBrightness(int brightness) {
    if (brightness < 0) {
        brightness = 0;
    }
    if (brightness > 100) {
        brightness = 100;
    }

    uint8_t params[2];
    writeUint16BE(params, (uint16_t)brightness);
    sendCrt("LLUM", params, sizeof(params));
}

void LibUSBHIDAPI::setKeyboardLightingEffects(int effect) {
    if (effect < 0) {
        effect = 0;
    }
    if (effect > 9) {
        effect = 9;
    }

    uint8_t params[2];
    writeUint16BE(params, (uint16_t)effect);
    sendCrt("LMOD", params, sizeof(params));
}

void LibUSBHIDAPI::setKeyboardLightingSpeed(int speed) {
    if (speed < 0) {
        speed = 0;
    }
    if (speed > 7) {
        speed = 7;
    }

    uint8_t params[2];
    writeUint16BE(params, (uint16_t)speed);
    sendCrt("LSPE", params, sizeof(params));
}

void LibUSBHIDAPI::setKeyboardRgbBacklight(int red, int green, int blue) {
    const uint8_t params[6] = {0, 0, 0, (uint8_t)red, (uint8_t)green, (uint8_t)blue};
    sendCrt("COLOR", params, sizeof(params));
}

void LibUSBHIDAPI::keyboardOsModeSwitch(int os_mode) {
    if (os_mode < 0) {
        os_mode = 0;
    }
    if (os_mode > 1) {
        os_mode = 1;
    }

    const uint16_t mode_char = (os_mode == 1) ? 'M' : 'W';
    uint8_t params[2];
    writeUint16BE(params, mode_char);
    sendCrt("CPOS", params, sizeof(params));
}

void LibUSBHIDAPI::setDeviceConfig(const uint8_t *configs, size_t config_len) {
    sendCrt("QUCMD", configs, config_len);
}

void LibUSBHIDAPI::changeMode(int mode) {
    const uint8_t params[1] = {(uint8_t)('1' + mode)};
    sendCrt("MOD", params, sizeof(params));
}

void LibUSBHIDAPI::changePage(int page) {
    const uint8_t params[1] = {(uint8_t)page};
    sendCrt("M_V", params, sizeof(params), nullptr, 0, nullptr, 0, true);
}

void LibUSBHIDAPI::setN1SkinBitmap(
    const uint8_t *png_data,
    size_t png_len,
    int skin_mode,
    int skin_page,
    int skin_status,
    int key_index,
    uint32_t timeout_ms
) {
    (void)timeout_ms;

    uint8_t params[8];
    writeUint32BE(params, (uint32_t)png_len);
    params[4] = (uint8_t)skin_mode;
    params[5] = (uint8_t)skin_page;
    params[6] = (uint8_t)skin_status;
    params[7] = (uint8_t)key_index;

    const uint8_t custom_crt[5] = {'C', 'R', 'T', 0xFF, 0x00};
    sendCrt("LOG", params, sizeof(params), png_data, png_len, custom_crt, sizeof(custom_crt));
}

void LibUSBHIDAPI::heartbeat() {
    sendCrt("CONNECT");
}

void LibUSBHIDAPI::notifyDisconnected() {
    uint8_t params[] = {0, 0, 'D', 'C'};
    sendCrt("CLE", params, sizeof(params));
}

void LibUSBHIDAPI::setReportId(uint8_t report_id) {
    report_id_ = report_id;
}

uint8_t LibUSBHIDAPI::getReportId() const {
    return report_id_;
}

void LibUSBHIDAPI::setReportSize(uint16_t input_report_size, uint16_t output_report_size, uint16_t feature_report_size) {
    input_report_size_ = input_report_size;
    output_report_size_ = output_report_size;
    feature_report_size_ = feature_report_size;
}

std::string LibUSBHIDAPI::getLastError() const {
    return last_error_;
}

void LibUSBHIDAPI::configureK1ProDefaults() {
    setReportSize(513, 1024, 0);
    setReportId(0x04);
}
