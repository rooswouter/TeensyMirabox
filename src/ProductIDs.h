#pragma once

#include <stdint.h>

/**
 * @file ProductIDs.h
 * @brief USB vendor and product IDs for StreamDock device detection.
 *
 * Mirrors `ProductIDs.py` from the Python SDK. Used by `DeviceManager` to
 * instantiate the correct `StreamDock` subclass.
 */

/** @brief USB vendor IDs (VID). */
struct USBVendorIDs {
    static constexpr uint16_t USB_VID_293 = 0x5500;
    static constexpr uint16_t USB_VID_293V3 = 0x6603;
    static constexpr uint16_t USB_VID_293V3EN = 0x6603;
    static constexpr uint16_t USB_VID_293s = 0x5548;
    static constexpr uint16_t USB_VID_293sV3 = 0x6603;
    static constexpr uint16_t USB_VIDN3 = 0x6603;
    static constexpr uint16_t USB_VIDN3V2 = 0xEEEF;
    static constexpr uint16_t USB_VIDN3V25 = 0x1500;
    static constexpr uint16_t USB_VIDN3E = 0x6602;
    static constexpr uint16_t USB_VIDN4 = 0x6602;
    static constexpr uint16_t USB_VIDN4EN = 0x6603;
    static constexpr uint16_t USB_VIDN1EN = 0x6603;
    static constexpr uint16_t USB_VIDN1 = 0x6603;
    static constexpr uint16_t USB_VID_N4PRO = 0x5548;
    static constexpr uint16_t USB_VID_N4PROEN = 0x5548;
    static constexpr uint16_t USB_VID_XL = 0x5548;
    static constexpr uint16_t USB_VID_XLEN = 0x5548;
    static constexpr uint16_t USB_VID_M18 = 0x6603;
    static constexpr uint16_t USB_VID_M18EN = 0x6603;
    static constexpr uint16_t USB_VID_M3 = 0x5548;
    static constexpr uint16_t USB_VID_M3EN = 0x5548;
    static constexpr uint16_t USB_VID_K1_PRO = 0x6603;
    static constexpr uint16_t USB_VID_K1_PROEU = 0x6603;
    static constexpr uint16_t USB_VID_Mini = 0x5548;
    static constexpr uint16_t USB_VID_MiniW = 0x5548;
};

/** @brief USB product IDs (PID). */
struct USBProductIDs {
    static constexpr uint16_t USB_PID_STREAMDOCK_293 = 0x1001;
    static constexpr uint16_t USB_PID_STREAMDOCK_293V3 = 0x1005;
    static constexpr uint16_t USB_PID_STREAMDOCK_293V3EN = 0x1006;
    static constexpr uint16_t USB_PID_STREAMDOCK_293V25 = 0x1010;
    static constexpr uint16_t USB_PID_STREAMDOCK_293s = 0x6670;
    static constexpr uint16_t USB_PID_STREAMDOCK_293sV3 = 0x1014;
    static constexpr uint16_t USB_PID_STREAMDOCK_N3 = 0x1002;
    static constexpr uint16_t USB_PID_STREAMDOCK_N3EN = 0x1003;
    static constexpr uint16_t USB_PID_STREAMDOCK_N3V2 = 0x2929;
    static constexpr uint16_t USB_PID_STREAMDOCK_N3V25 = 0x3001;
    static constexpr uint16_t USB_PID_STREAMDOCK_N4 = 0x1001;
    static constexpr uint16_t USB_PID_STREAMDOCK_N4EN = 0x1007;
    static constexpr uint16_t USB_PID_STREAMDOCK_N1EN = 0x1000;
    static constexpr uint16_t USB_PID_STREAMDOCK_N1 = 0x1011;
    static constexpr uint16_t USB_PID_STREAMDOCK_N4PRO = 0x1008;
    static constexpr uint16_t USB_PID_STREAMDOCK_N4PROEN = 0x1021;
    static constexpr uint16_t USB_PID_STREAMDOCK_XL = 0x1028;
    static constexpr uint16_t USB_PID_STREAMDOCK_XLEN = 0x1031;
    static constexpr uint16_t USB_PID_STREAMDOCK_M18 = 0x1009;
    static constexpr uint16_t USB_PID_STREAMDOCK_M18EN = 0x1012;
    static constexpr uint16_t USB_PID_STREAMDOCK_M3 = 0x1020;
    static constexpr uint16_t USB_PID_STREAMDOCK_M3EN = 0x1032;
    static constexpr uint16_t USB_PID_K1_PRO = 0x1015;
    static constexpr uint16_t USB_PID_K1_PROEU = 0x1019;
    static constexpr uint16_t USB_PID_Mini = 0x1036;
    static constexpr uint16_t USB_PID_MiniW = 0x1037;
};
