#pragma once

#include <USBHost_t36.h>
#include <stdint.h>
#include <stddef.h>
#include <string>

#include "MiraBoxHIDInput.h"
#include "Transport/LibUSBHIDAPI.h"

#include "Devices/K1Pro.h"
#include "Devices/StreamDock.h"

// Teensy-friendly equivalent of the Python DeviceManager:
// - no background threads
// - hotplug is detected via polling (meant to be called from Arduino `loop()`)
// - each slot owns USBHIDParser instances required for USBHost_t36 HID claiming

class DeviceManager {
public:
    using DeviceCallback = void (*)(StreamDock *);

    explicit DeviceManager(USBHost &host);

    void setDeviceChangeCallback(DeviceCallback on_device_added, DeviceCallback on_device_removed);

    // After begin(), call poll() regularly (e.g. from Arduino loop()).
    // The sketch must also call USBHost::begin() once in setup().
    void begin(bool auto_open = true, bool auto_init = false, uint32_t poll_interval_ms = 250);

    void poll();

    size_t deviceCount() const;
    StreamDock *deviceAt(size_t idx) const;

private:
    static constexpr uint8_t MAX_SLOTS = 1;

    struct Slot {
        USBHIDParser hid_parser_1;
        USBHIDParser hid_parser_2;
        USBHIDParser hid_parser_3;
        MiraBoxHIDInput keyboard;
        MiraBoxHIDInput control;
        LibUSBHIDAPI transport;
        StreamDock *device = nullptr;
        bool control_active = false;
        uint8_t slot_index = 0;

        Slot(USBHost &host, uint8_t slotIndex);

        bool isControlConnected() const;
        void fillHidDeviceInfo(HidDeviceInfo &out) const;
    };

    USBHost &host_;
    Slot slots_[MAX_SLOTS];

    DeviceCallback on_device_added_ = nullptr;
    DeviceCallback on_device_removed_ = nullptr;

    bool auto_open_ = true;
    bool auto_init_ = false;
    uint32_t poll_interval_ms_ = 250;
    uint32_t last_poll_ms_ = 0;

    StreamDock *createDevice(uint16_t vendor_id, uint16_t product_id, LibUSBHIDAPI &transport, const HidDeviceInfo &info) const;
    void onControlConnected(Slot &slot);
    void onControlDisconnected(Slot &slot);
};
