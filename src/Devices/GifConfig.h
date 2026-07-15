#pragma once

/**
 * @file GifConfig.h
 * @brief Compile-time limits for GIF decode/encode (WITH_ANIMATEDGIF).
 */

#ifndef GIF_MAX_FILE_BYTES
#define GIF_MAX_FILE_BYTES (512UL * 1024UL)
#endif

#ifndef GIF_MAX_FRAMES
#define GIF_MAX_FRAMES 64
#endif

/** AnimatedGIF uses ucLineBuf[MAX_WIDTH] (480 on Teensy) — see AnimatedGIF.h */
#ifndef GIF_MAX_CANVAS_WIDTH
#define GIF_MAX_CANVAS_WIDTH 480
#endif

#ifndef GIF_MAX_CANVAS_HEIGHT
#define GIF_MAX_CANVAS_HEIGHT 480
#endif

#ifndef GIF_JPEG_QUALITY
#define GIF_JPEG_QUALITY 1
#endif
