#pragma once

#include <cstddef>
#include <cstdint>

class StreamDock;

class GifController {
public:
    static constexpr int BACKGROUND_INDEX = 0;
    static constexpr int DEFAULT_DELAY_MS = 100;

    explicit GifController(StreamDock &device);

    int set_key_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int key);
    int clear_key_gif(int key);
    int set_background_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int x = 0, int y = 0, uint8_t fb_layer = 0x00);
    int clear_background_gif(int position = 0x03);
    int clear_background_animation(int position = 0x03);
    int start_gif_loop();
    int stop_gif_loop();
    bool gif_loop_status() const;
    int start_animation_loop();
    int stop_animation_loop();
    bool animation_loop_status() const;
    void close();
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
