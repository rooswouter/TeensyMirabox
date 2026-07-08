#pragma once

#include <cstdint>

enum class ImageFileFormat : uint8_t {
    JPEG,
    PNG,
};

struct ImageFormat {
    uint16_t width = 0;
    uint16_t height = 0;
    ImageFileFormat format = ImageFileFormat::JPEG;
    int16_t rotation = 0;
    bool flip_x = false;
    bool flip_y = false;
};
