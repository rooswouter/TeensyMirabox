#pragma once

#include <cstddef>
#include <cstdint>

class StreamDock;

/**
 * @file GifController.h
 * @brief In-memory GIF animation scheduler for StreamDock keys and backgrounds.
 */

/**
 * @class GifController
 * @brief Cycles JPEG frames on keys or background layers when `update()` is called.
 */
class GifController {
public:
    static constexpr int BACKGROUND_INDEX = 0;
    static constexpr int DEFAULT_DELAY_MS = 100;

    /**
     * @brief Attach to a StreamDock device for image uploads.
     * @param device Parent device used to call `set_key_imageData` / background APIs.
     */
    explicit GifController(StreamDock &device);

    /**
     * @brief Register a multi-frame GIF for one key.
     * @return 0 on success, negative on error.
     */
    int set_key_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int key);

    /** @brief Remove GIF animation from a key. */
    int clear_key_gif(int key);

    /** @brief Register a background GIF at an optional offset and layer. */
    int set_background_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int x = 0, int y = 0, uint8_t fb_layer = 0x00);

    /** @brief Stop background GIF on a layer. */
    int clear_background_gif(int position = 0x03);

    /** @brief Alias for `clear_background_gif`. */
    int clear_background_animation(int position = 0x03);

    /** @brief Enable GIF frame advancement in `update()`. */
    int start_gif_loop();

    /** @brief Disable GIF frame advancement. */
    int stop_gif_loop();

    /** @return True if the GIF loop flag is set. */
    bool gif_loop_status() const;

    /** @brief Enable background animation loop (alias). */
    int start_animation_loop();

    /** @brief Disable background animation loop (alias). */
    int stop_animation_loop();

    /** @return True if animation loop is active. */
    bool animation_loop_status() const;

    /** @brief Stop all streams and reset state. */
    void close();

    /**
     * @brief Advance animations and upload the next frame when due.
     *
     * Called automatically from `StreamDock::poll()`.
     */
    void update();

private:
    struct GifStreamStatus {
        const uint8_t *const *frames = nullptr;
        const size_t *frame_sizes = nullptr;
        const int *delays = nullptr;
        size_t frame_count = 0;
        size_t current_frame = 0;
        unsigned long accumulated_ms = 0;
        int width = 0;
        int height = 0;
        int x = 0;
        int y = 0;
        uint8_t fb_layer = 0;
        bool active = false;
    };

    int get_key_values(int key, int &hardware_key) const;
    void replace_stream(int index, const GifStreamStatus &status);
    void remove_stream(int index);
    GifStreamStatus *find_stream(int index);

    StreamDock &device_;
    GifStreamStatus streams_[8];
    int stream_indices_[8] = {-1, -1, -1, -1, -1, -1, -1, -1};
    size_t stream_count_ = 0;
    bool loop_enabled_ = false;
    bool running_ = true;
};
