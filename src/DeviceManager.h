#pragma once

#include <USBHost_t36.h>
#include <stdint.h>
#include <stddef.h>
#include <string>

#include "MiraBoxHIDInput.h"
#include "Transport/LibUSBHIDAPI.h"

#include "Devices/K1Pro.h"
#include "Devices/StreamDock.h"

/**
 * @file DeviceManager.h
 * @brief Teensy USB-host device manager for StreamDock / Mirabox hardware.
 *
 * Poll-based equivalent of the Python `DeviceManager`: detects connect/disconnect,
 * creates the correct `StreamDock` subclass, and drives `USBHost::Task()` plus
 * per-device `poll()` from your Arduino `loop()`.
 *
 * Your sketch must call `USBHost::begin()` once in `setup()` in addition to
 * `DeviceManager::begin()`.
 */

/**
 * @class DeviceManager
 * @brief Detects, opens, and polls connected StreamDock devices over USB Host.
 */
class DeviceManager {
public:
    /** @brief Callback invoked when a device is connected and ready. */
    using DeviceCallback = void (*)(StreamDock *);

    /**
     * @brief Construct a device manager bound to a USBHost instance.
     * @param host USB host object declared in the sketch (e.g. `USBHost myusb`).
     */
    explicit DeviceManager(USBHost &host);

    /**
     * @brief Register hotplug callbacks.
     * @param on_device_added Called after a device is opened (and optionally initialized).
     * @param on_device_removed Called before the device is closed and destroyed.
     */
    void setDeviceChangeCallback(DeviceCallback on_device_added, DeviceCallback on_device_removed);

    /**
     * @brief Configure the manager and reset debug flags.
     * @param auto_open If true, call `StreamDock::open()` when a device connects.
     * @param auto_init If true, call `StreamDock::init()` after open (skipped for K1 Pro).
     * @param poll_interval_ms Minimum interval between hotplug checks (default 250 ms).
     *
     * Call `poll()` regularly from `loop()` after `USBHost::begin()`.
     */
    void begin(bool auto_open = true, bool auto_init = false, uint32_t poll_interval_ms = 250);

    /**
     * @brief Run USB host tasks, poll connected devices, and detect hotplug.
     *
     * Must be called frequently from `loop()`. Internally calls `host_.Task()`.
     */
    void poll();

    /**
     * @brief Number of currently managed devices.
     * @return Count of connected StreamDock instances (0 or 1 with current MAX_SLOTS).
     */
    size_t deviceCount() const;

    /**
     * @brief Access a connected device by index.
     * @param idx Zero-based index (0 .. deviceCount()-1).
     * @return Pointer to the device, or `nullptr` if index is out of range.
     */
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
    USBHub hub_;
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
