#include "DeviceConfigEvent.h"
#include <ArduinoJson.h>

namespace {

std::string jsonString(const JsonVariantConst &value) {
    const char *text = value.as<const char *>();
    return text ? text : "";
}

void copyIntArray(const JsonArray &array, int *dest, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        dest[i] = (i < array.size()) ? array[i].as<int>() : 0;
    }
}

const uint8_t *jsonPayload(const uint8_t *buffer, uint32_t length, uint32_t &json_length) {
    const uint8_t *json_start = buffer;
    json_length = length;

    if (length >= 12
        && buffer[0] == 0x04
        && buffer[1] == 'D'
        && buffer[2] == 'E'
        && buffer[3] == 'V'
        && buffer[4] == 'C'
        && buffer[5] == 'F'
        && buffer[6] == 'G') {
        json_start = buffer + 12;
        json_length = length - 12;
    }

    while (json_length > 0 && json_start[json_length - 1] == 0) {
        --json_length;
    }

    return json_start;
}

} // namespace

DeviceConfigEvent::DeviceConfigEvent(const uint8_t *buffer, uint32_t length) {
    uint32_t json_length = 0;
    const uint8_t *json_start = jsonPayload(buffer, length, json_length);
    if (json_length == 0) {
        return;
    }

    JsonDocument doc;
    const DeserializationError error = deserializeJson(doc, json_start, json_length);
    if (error) {
        return;
    }

    version = jsonString(doc["version"]);
    os = jsonString(doc["os"]);
    scr = doc["scr"] | 0;
    style = doc["style"] | 0;
    Stream_Dock = jsonString(doc["Stream Dock"]);

    const JsonObject led = doc["led_info"];
    if (!led.isNull()) {
        led_info.mode = led["mode"] | 0;
        led_info.speed = led["speed"] | 0;
        led_info.brightness = led["brightness"] | 0;
        led_info.flag = led["flag"] | 0;
        copyIntArray(led["hsv"], led_info.hsv, 3);
        copyIntArray(led["base_hs"], led_info.base_hs, 2);
    }

    valid = true;
}
