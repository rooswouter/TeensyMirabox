#pragma once

#include "StreamDock.h"

/**
 * @file DeviceHelpers.h
 * @brief Shared helpers for device drivers and HID report setup.
 */

/**
 * @brief Upload one JPEG to a key using logical-to-hardware key mapping.
 * @param device Target StreamDock instance.
 * @param key Logical key index (1-based).
 * @param data JPEG bytes.
 * @param length Data length.
 * @return 0 on success, -1 on invalid key or mapping.
 */
int setDualKeyImage(StreamDock &device, int key, const uint8_t *data, size_t length);

/**
 * @brief Configure transport for 1025-byte output reports (report ID 0).
 * @param transport HID transport to configure.
 */
void configureReport1025(LibUSBHIDAPI &transport);

/**
 * @brief Configure transport for 513-byte output reports (report ID 0).
 * @param transport HID transport to configure.
 */
void configureReport513(LibUSBHIDAPI &transport);
