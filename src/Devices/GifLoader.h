#pragma once


#include <cstddef>
#include <cstdint>
#define ENABLE_ANIMATEDGIF

class StreamDock;
class GifController;

/**
 * @file GifLoader.h
 * @brief Decode GIF files to JPEG frames for GifController playback.
 *
 * Requires `ENABLE_ANIMATEDGIF` and the AnimatedGIF + JPEGENC libraries.
 * When the flag is undefined, all load methods return -1.
 */

class GifLoader {
public:
    GifLoader(StreamDock &device, GifController &gif_controller);

    int loadKeyGif(int key, const uint8_t *data, size_t length);
    int loadBackgroundGif(const uint8_t *data, size_t length, int x, int y, uint8_t fb_layer);
    int loadKeyGifFile(int key, const char *sd_path);
    int loadBackgroundGifFile(const char *sd_path, int x, int y, uint8_t fb_layer);

    void releaseKey(int key);
    void releaseBackground();
    void releaseAll();

    void onStreamRemoved(int stream_index);

    static void streamRemovedThunk(int stream_index, void *context);

private:
    struct OwnedGifStream {
        uint8_t **frames = nullptr;
        size_t *frame_sizes = nullptr;
        int *delays = nullptr;
        size_t frame_count = 0;
        bool active = false;
    };

    static constexpr size_t kMaxOwnedStreams = 8;

    StreamDock &device_;
    GifController &gif_controller_;
    OwnedGifStream entries_[kMaxOwnedStreams];
    int entry_indices_[kMaxOwnedStreams] = {-1, -1, -1, -1, -1, -1, -1, -1};

    OwnedGifStream *findEntry(int stream_index);
    const OwnedGifStream *findEntry(int stream_index) const;
    void freeEntry(OwnedGifStream &entry);
    void freePartialEntry(OwnedGifStream &entry, size_t decoded_frame_count);
    int storeEntry(int stream_index, OwnedGifStream &entry);
    void removeEntry(int stream_index);

#ifdef ENABLE_ANIMATEDGIF
    int decodeGifToStream(const uint8_t *data, size_t length, int target_width, int target_height, OwnedGifStream &out);
#endif
};
