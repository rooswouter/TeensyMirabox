#include "DeviceManager.h"

#include <Arduino.h>
#include <cstdio>

#include "ProductIDs.h"

#include "Devices/Devices.h"

static std::string safeToString(const uint8_t *s) {
    if (s == nullptr) return std::string{};
    return std::string(reinterpret_cast<const char *>(s));
}


DeviceManager::Slot::Slot(USBHost &host, uint8_t slotIndex)
    : hid_parser_1(host),
      hid_parser_2(host),
      hid_parser_3(host),
      keyboard(host, 1),
      control(host, 2),
      transport(control),
      slot_index(slotIndex) {}

bool DeviceManager::Slot::isControlConnected() const {
    return control.isConnected();
}

void DeviceManager::Slot::fillHidDeviceInfo(HidDeviceInfo &out) {
    out.vendor_id = control.idVendor();
    out.product_id = control.idProduct();
    out.serial_number = safeToString(control.serialNumber());
    out.manufacturer_string = safeToString(control.manufacturer());
    out.product_string = safeToString(control.product());

    if (!out.serial_number.empty()) {
        out.path = out.serial_number;
    } else {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "usb-slot-%u", static_cast<unsigned>(slot_index));
        out.path = buf;
    }
}

DeviceManager::DeviceManager(USBHost &host)
    : host_(host),
      hub_(host),
      slots_{Slot(host_, 0)} {}

void DeviceManager::setDeviceChangeCallback(DeviceCallback on_device_added, DeviceCallback on_device_removed) {
    on_device_added_ = on_device_added;
    on_device_removed_ = on_device_removed;
}

void DeviceManager::begin(bool auto_open, bool auto_init, uint32_t poll_interval_ms) {
    auto_open_ = auto_open;
    auto_init_ = auto_init;
    poll_interval_ms_ = poll_interval_ms;
    last_poll_ms_ = 0;

    MiraBoxHIDInput::show_raw_data = false;
    MiraBoxHIDInput::show_formated_data = false;
}

size_t DeviceManager::deviceCount() const {
    size_t count = 0;
    for (uint8_t i = 0; i < MAX_SLOTS; ++i) {
        if (slots_[i].device != nullptr) ++count;
    }
    return count;
}

StreamDock *DeviceManager::deviceAt(size_t idx) const {
    size_t seen = 0;
    for (uint8_t i = 0; i < MAX_SLOTS; ++i) {
        if (slots_[i].device == nullptr) continue;
        if (seen == idx) return slots_[i].device;
        ++seen;
    }
    return nullptr;
}

StreamDock *DeviceManager::createDevice(uint16_t vendor_id,
                                        uint16_t product_id,
                                        LibUSBHIDAPI &transport,
                                        const HidDeviceInfo &info) const {
    if (vendor_id == USBVendorIDs::USB_VID_293 && product_id == USBProductIDs::USB_PID_STREAMDOCK_293) {
        return new StreamDock293(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_293V3 && product_id == USBProductIDs::USB_PID_STREAMDOCK_293V3) {
        return new StreamDock293V3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_293V3EN && product_id == USBProductIDs::USB_PID_STREAMDOCK_293V3EN) {
        return new StreamDock293V3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_293V3 && product_id == USBProductIDs::USB_PID_STREAMDOCK_293V25) {
        return new StreamDock293V3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_293s && product_id == USBProductIDs::USB_PID_STREAMDOCK_293s) {
        return new StreamDock293s(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_293sV3 && product_id == USBProductIDs::USB_PID_STREAMDOCK_293sV3) {
        return new StreamDock293sV3(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VIDN3 && product_id == USBProductIDs::USB_PID_STREAMDOCK_N3) {
        return new StreamDockN3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VIDN3 && product_id == USBProductIDs::USB_PID_STREAMDOCK_N3EN) {
        return new StreamDockN3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VIDN3E && product_id == USBProductIDs::USB_PID_STREAMDOCK_N3) {
        return new StreamDockN3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VIDN3E && product_id == USBProductIDs::USB_PID_STREAMDOCK_N3EN) {
        return new StreamDockN3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VIDN3E && product_id == USBProductIDs::USB_PID_STREAMDOCK_N3V2) {
        return new StreamDockN3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VIDN3V25 && product_id == USBProductIDs::USB_PID_STREAMDOCK_N3V25) {
        return new StreamDockN3(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VIDN4 && product_id == USBProductIDs::USB_PID_STREAMDOCK_N4) {
        return new StreamDockN4(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VIDN4EN && product_id == USBProductIDs::USB_PID_STREAMDOCK_N4EN) {
        return new StreamDockN4(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VIDN1 && product_id == USBProductIDs::USB_PID_STREAMDOCK_N1) {
        return new StreamDockN1(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VIDN1EN && product_id == USBProductIDs::USB_PID_STREAMDOCK_N1EN) {
        return new StreamDockN1(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VID_N4PRO && product_id == USBProductIDs::USB_PID_STREAMDOCK_N4PRO) {
        return new StreamDockN4Pro(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_N4PROEN && product_id == USBProductIDs::USB_PID_STREAMDOCK_N4PROEN) {
        return new StreamDockN4Pro(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VID_XL && product_id == USBProductIDs::USB_PID_STREAMDOCK_XL) {
        return new StreamDockXL(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_XLEN && product_id == USBProductIDs::USB_PID_STREAMDOCK_XLEN) {
        return new StreamDockXL(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VID_M18 && product_id == USBProductIDs::USB_PID_STREAMDOCK_M18) {
        return new StreamDockM18(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_M18EN && product_id == USBProductIDs::USB_PID_STREAMDOCK_M18EN) {
        return new StreamDockM18(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VID_M3 && product_id == USBProductIDs::USB_PID_STREAMDOCK_M3) {
        return new StreamDockM3(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_M3EN && product_id == USBProductIDs::USB_PID_STREAMDOCK_M3EN) {
        return new StreamDockM3(transport, info);
    }

    if (isK1Pro(vendor_id, product_id)) {
        return new K1Pro(transport, info);
    }

    if (vendor_id == USBVendorIDs::USB_VID_Mini && product_id == USBProductIDs::USB_PID_Mini) {
        return new StreamDockMini(transport, info);
    }
    if (vendor_id == USBVendorIDs::USB_VID_MiniW && product_id == USBProductIDs::USB_PID_MiniW) {
        return new StreamDockMini(transport, info);
    }

    return nullptr;
}

void DeviceManager::onControlConnected(Slot &slot) {
    HidDeviceInfo info;
    slot.fillHidDeviceInfo(info);

    if (isK1Pro(info.vendor_id, info.product_id)) {
        slot.transport.configureK1ProDefaults();
    }

    StreamDock *dev = createDevice(info.vendor_id, info.product_id, slot.transport, info);
    if (dev == nullptr) {
        Serial.printf("DeviceManager: unsupported device %04X:%04X\n", info.vendor_id, info.product_id);
        return;
    }

    slot.device = dev;

    // Configure transport parameters and device type now (no USB traffic).
    // The added callback relies on get_device_type() being valid even for
    // devices whose init() is deferred (K1Pro).
    dev->set_device();

    if (auto_open_) {
        if (!slot.device->open()) {
            delete dev;
            slot.device = nullptr;
            Serial.println("DeviceManager: failed to open device");
            return;
        }
    }

    if (isK1Pro(info.vendor_id, info.product_id)) {
        // The K1Pro ignores (NAKs) commands sent right after enumeration, which
        // wedges the interrupt OUT pipe and blocks every later write. Give it
        // the same settle time the k1pro.ino bring-up sequence uses.
        const uint32_t settle_deadline = millis() + 500;
        while ((int32_t)(millis() - settle_deadline) < 0) {
            host_.Task();
            yield();
        }
    } else if (auto_open_ && auto_init_) {
        // K1Pro is initialized via keyboard_mode(1) from the added callback;
        // running init() here as well would do it twice and too early.
        slot.device->init();
    }

    if (on_device_added_ != nullptr) {
        on_device_added_(slot.device);
    }
}

void DeviceManager::onControlDisconnected(Slot &slot) {
    if (slot.device == nullptr) {
        return;
    }

    if (on_device_removed_ != nullptr) {
        on_device_removed_(slot.device);
    }

    StreamDock *dev = slot.device;
    slot.device = nullptr;
    dev->close(false);
    delete dev;
}

void DeviceManager::poll() {
    host_.Task();

    for (uint8_t slot_idx = 0; slot_idx < MAX_SLOTS; ++slot_idx) {
        Slot &slot = slots_[slot_idx];
        if (slot.control_active && slot.device != nullptr) {
            slot.device->poll();
        }
    }

    const uint32_t now = millis();
    if (now - last_poll_ms_ < poll_interval_ms_) {
        return;
    }
    last_poll_ms_ = now;

    for (uint8_t slot_idx = 0; slot_idx < MAX_SLOTS; ++slot_idx) {
        Slot &slot = slots_[slot_idx];
        const bool connected = slot.isControlConnected();

        if (connected != slot.control_active) {
            slot.control_active = connected;
            if (connected) {
                onControlConnected(slot);
            } else {
                onControlDisconnected(slot);
            }
        }
    }
}

bool DeviceManager::isK1Pro(uint16_t vendor_id, uint16_t product_id) {
    return (vendor_id == USBVendorIDs::USB_VID_K1_PRO && product_id == USBProductIDs::USB_PID_K1_PRO)
        || (vendor_id == USBVendorIDs::USB_VID_K1_PROEU && product_id == USBProductIDs::USB_PID_K1_PROEU);
}

