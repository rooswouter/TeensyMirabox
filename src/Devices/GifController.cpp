#include "GifController.h"
#include "StreamDock.h"

#include <Arduino.h>

GifController::GifController(StreamDock &device) : device_(device) {}

int GifController::get_key_values(int key, int &hardware_key) const {
    if (key < 1) {
        return -1;
    }
    hardware_key = device_.get_image_key(static_cast<ButtonKey>(key));
    return hardware_key >= 0 ? 0 : -1;
}

GifController::GifStreamStatus *GifController::find_stream(int index) {
    for (size_t i = 0; i < stream_count_; ++i) {
        if (stream_indices_[i] == index) {
            return &streams_[i];
        }
    }
    return nullptr;
}

void GifController::replace_stream(int index, const GifStreamStatus &status) {
    if (GifStreamStatus *existing = find_stream(index)) {
        if (stream_removed_callback_ != nullptr) {
            stream_removed_callback_(index, stream_removed_context_);
        }
        *existing = status;
        return;
    }
    if (stream_count_ < 8) {
        streams_[stream_count_] = status;
        stream_indices_[stream_count_] = index;
        ++stream_count_;
    }
}

void GifController::remove_stream(int index) {
    for (size_t i = 0; i < stream_count_; ++i) {
        if (stream_indices_[i] == index) {
            if (stream_removed_callback_ != nullptr) {
                stream_removed_callback_(index, stream_removed_context_);
            }
            for (size_t j = i + 1; j < stream_count_; ++j) {
                streams_[j - 1] = streams_[j];
                stream_indices_[j - 1] = stream_indices_[j];
            }
            --stream_count_;
            return;
        }
    }
}

int GifController::set_key_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int key) {
    if (!device_.feature_option.supportKeyGif) {
        return -1;
    }

    int hardware_key = 0;
    if (get_key_values(key, hardware_key) != 0 || frame_count == 0) {
        return -1;
    }

    GifStreamStatus status;
    status.frames = frames;
    status.frame_sizes = frame_sizes;
    status.delays = delays;
    status.frame_count = frame_count;
    status.current_frame = frame_count > 0 ? frame_count - 1 : 0;
    status.accumulated_ms = 0;
    status.active = true;
    replace_stream(hardware_key, status);
    return 0;
}

int GifController::clear_key_gif(int key) {
    int hardware_key = 0;
    if (get_key_values(key, hardware_key) != 0) {
        return -1;
    }
    remove_stream(hardware_key);
    return 0;
}

int GifController::set_background_gif_stream(const uint8_t *const *frames, const size_t *frame_sizes, const int *delays, size_t frame_count, int x, int y, uint8_t fb_layer) {
    if (!device_.feature_option.supportBackgroundGif || frame_count == 0) {
        return -1;
    }

    const ImageFormat format = device_.touchscreen_image_format();
    GifStreamStatus status;
    status.frames = frames;
    status.frame_sizes = frame_sizes;
    status.delays = delays;
    status.frame_count = frame_count;
    status.current_frame = frame_count > 0 ? frame_count - 1 : 0;
    status.width = format.width;
    status.height = format.height;
    status.x = x;
    status.y = y;
    status.fb_layer = fb_layer;
    status.active = true;
    replace_stream(BACKGROUND_INDEX, status);
    return 0;
}

int GifController::clear_background_gif(int position) {
    return clear_background_animation(position);
}

int GifController::clear_background_animation(int position) {
    if (!device_.feature_option.supportBackgroundGif) {
        return -1;
    }
    remove_stream(BACKGROUND_INDEX);
    device_.transport.clearBackgroundFrameStream(position);
    return 0;
}

int GifController::start_gif_loop() {
    return start_animation_loop();
}

int GifController::stop_gif_loop() {
    return stop_animation_loop();
}

bool GifController::gif_loop_status() const {
    return animation_loop_status();
}

int GifController::start_animation_loop() {
    loop_enabled_ = true;
    return 0;
}

int GifController::stop_animation_loop() {
    loop_enabled_ = false;
    return 0;
}

bool GifController::animation_loop_status() const {
    return loop_enabled_;
}

void GifController::close() {
    while (stream_count_ > 0) {
        const int index = stream_indices_[stream_count_ - 1];
        if (stream_removed_callback_ != nullptr) {
            stream_removed_callback_(index, stream_removed_context_);
        }
        --stream_count_;
    }
    running_ = false;
    loop_enabled_ = false;
}

void GifController::set_stream_removed_callback(StreamRemovedCallback callback, void *context) {
    stream_removed_callback_ = callback;
    stream_removed_context_ = context;
}

void GifController::update() {
    if (!running_ || !loop_enabled_ || stream_count_ == 0) {
        return;
    }

    static unsigned long last_update_ms = 0;
    const unsigned long now = millis();
    const unsigned long elapsed = now - last_update_ms;
    last_update_ms = now;

    bool refreshed = false;
    for (size_t i = 0; i < stream_count_; ++i) {
        GifStreamStatus &gif = streams_[i];
        if (!gif.active || gif.frame_count == 0 || gif.frames == nullptr || gif.delays == nullptr || gif.frame_sizes == nullptr) {
            continue;
        }

        gif.accumulated_ms += elapsed;
        const int delay = gif.delays[gif.current_frame] > 0 ? gif.delays[gif.current_frame] : DEFAULT_DELAY_MS;
        if (gif.accumulated_ms < (unsigned long)delay) {
            continue;
        }

        gif.accumulated_ms -= delay;
        gif.current_frame = (gif.current_frame + 1) % gif.frame_count;
        const uint8_t *frame = gif.frames[gif.current_frame];
        const size_t frame_size = gif.frame_sizes[gif.current_frame];
        if (frame == nullptr || frame_size == 0) {
            continue;
        }

        const int index = stream_indices_[i];
        if (index == BACKGROUND_INDEX) {
            device_.transport.setBackgroundFrameStream(frame, frame_size, gif.width, gif.height, gif.x, gif.y, gif.fb_layer);
        } else {
            device_.transport.setKeyImageStream(frame, frame_size, index);
        }
        refreshed = true;
    }

    if (refreshed) {
        device_.refresh();
    }
}
