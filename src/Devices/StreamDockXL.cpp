#include "StreamDockXL.h"
#include "DeviceKeyMaps.h"
#include "DeviceHelpers.h"

StreamDockXL::StreamDockXL(LibUSBHIDAPI &transport, const HidDeviceInfo &dev_info)
    : StreamDock(transport, dev_info) {
    config = new StreamDockXLConfig();
}

void StreamDockXL::set_device() {
    configureReport1025(transport);
    feature_option.hasRGBLed = true;
    feature_option.ledCounts = 6;
    feature_option.supportBackgroundGif = true;
    feature_option.supportConfig = true;
    feature_option.deviceType = device_type::dock_xl;
}

int StreamDockXL::set_brightness(int percent) {
    transport.setKeyBrightness(percent);
    return 0;
}

int StreamDockXL::set_key_imageData(int key, const uint8_t *data, size_t length) {
    return setDualKeyImage(*this, key, data, length);
}

int StreamDockXL::set_touchscreen_image(const uint8_t *data, size_t length) {
    transport.setBackgroundImageStream(data, length);
    return 0;
}

int StreamDockXL::get_image_key(ButtonKey logical_key) {
    return DeviceKeyMaps::getImageKey(DeviceKeyMaps::REMAP_XL_32, DeviceKeyMaps::REMAP_XL_32_COUNT, logical_key);
}

InputEvent StreamDockXL::decode_input_event(int hardware_code, int state) {
    static const DeviceKeyMaps::KnobRotateEntry rotate_map[] = {
        {0x23, KnobId::KNOB_1, Direction::LEFT}, {0x21, KnobId::KNOB_1, Direction::RIGHT},
        {0x24, KnobId::KNOB_2, Direction::LEFT}, {0x26, KnobId::KNOB_2, Direction::RIGHT},
    };
    InputEvent event = DeviceKeyMaps::decodeKnobRotate(rotate_map, sizeof(rotate_map) / sizeof(rotate_map[0]), hardware_code);
    if (event.event_type != EventType::UNKNOWN) {
        return event;
    }
    return DeviceKeyMaps::decodeButtonMap(DeviceKeyMaps::REMAP_XL_32, DeviceKeyMaps::REMAP_XL_32_COUNT, hardware_code, state);
}

ImageFormat StreamDockXL::key_image_format() const {
    return {80, 80, ImageFileFormat::PNG, 180, false, false};
}

ImageFormat StreamDockXL::touchscreen_image_format() const {
    return {1024, 600, ImageFileFormat::JPEG, 180, false, false};
}

int StreamDockXL::set_frame_background(const uint8_t *data, size_t length, int width, int height) {
    transport.setBackgroundFrameStream(data, length, width, height, 0, 0, 0);
    return 0;
}
