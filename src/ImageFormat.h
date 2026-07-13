#pragma once

#include <cstdint>

/** @brief Image file encoding expected by the device firmware. */
enum class ImageFileFormat : uint8_t {
    JPEG,
    PNG,
};

/**
 * @brief Describes required image dimensions and transforms for a device surface.
 */
struct ImageFormat {
    uint16_t width = 0;
    uint16_t height = 0;
    ImageFileFormat format = ImageFileFormat::JPEG;
    int16_t rotation = 0;   /**< Degrees clockwise; negative = counter-clockwise. */
    bool flip_x = false;
    bool flip_y = false;
};
