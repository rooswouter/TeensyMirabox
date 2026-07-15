#pragma once

// Maximum number of GIF streams (including shared streams) to be used by the GifLoader.
#define MAX_STREAMS 16
#include <cstddef>
#include <cstdint>

class StreamDock;
class GifController;

/**
 * @file GifLoader.h
 * @brief Decode GIF files to JPEG frames for GifController playback.
 *
 * Requires `WITH_ANIMATEDGIF` and the AnimatedGIF + JPEGENC libraries.
 * When the flag is undefined, all load methods return -1.
 */

/**
 * @brief Non-owning handle to decoded GIF frame data shared across GifLoader instances.
 *
 * Obtain via `GifLoader::exportKeyStream()` or `exportBackgroundStream()`.
 * Pass to `loadKeyGifShared()` / `loadBackgroundGifShared()` on another loader.
 * Frame buffers are freed when the last reference is released.
 */
class GifSharedStream {
public:
    bool valid() const { return data_ != nullptr && data_->frame_count > 0; }
    size_t frame_count() const { return data_ != nullptr ? data_->frame_count : 0; }

private:
    friend class GifLoader;

    struct GifStreamData {
        uint8_t **frames = nullptr;
        size_t *frame_sizes = nullptr;
        int *delays = nullptr;
        size_t frame_count = 0;
        uint32_t ref_count = 0;
    };

    GifStreamData *data_ = nullptr;
};

class GifLoader {
public:
    GifLoader(StreamDock &device, GifController &gif_controller);

    int loadKeyGif(int key, const uint8_t *data, size_t length);
    int loadBackgroundGif(const uint8_t *data, size_t length, int x, int y, uint8_t fb_layer);
    int loadKeyGifFile(int key, const char *sd_path);
    int loadBackgroundGifFile(const char *sd_path, int x, int y, uint8_t fb_layer);

    /** @brief Reuse decoded frames from another GifLoader without copying. */
    int loadKeyGifShared(int key, const GifSharedStream &shared);

    /** @brief Reuse decoded background frames from another GifLoader without copying. */
    int loadBackgroundGifShared(const GifSharedStream &shared, int x = 0, int y = 0, uint8_t fb_layer = 0x00);

    /** @brief Export a loaded key GIF for sharing. Does not change reference count. */
    GifSharedStream exportKeyStream(int key) const;

    /** @brief Export a loaded background GIF for sharing. Does not change reference count. */
    GifSharedStream exportBackgroundStream() const;

    void releaseKey(int key);
    void releaseBackground();
    void releaseAll();

    void onStreamRemoved(int stream_index);

    static void streamRemovedThunk(int stream_index, void *context);


private:
    struct StreamEntry {
        GifSharedStream::GifStreamData *data = nullptr;
        bool active = false;
    };


    StreamDock &device_;
    GifController &gif_controller_;
    StreamEntry entries_[MAX_STREAMS];
    int entry_indices_[MAX_STREAMS];

    StreamEntry *findEntry(int stream_index);
    const StreamEntry *findEntry(int stream_index) const;
    void freeStreamData(GifSharedStream::GifStreamData *data);
    void retainStreamData(GifSharedStream::GifStreamData *data);
    void releaseStreamData(GifSharedStream::GifStreamData *data);
    void freePartialStreamData(GifSharedStream::GifStreamData *data, size_t decoded_frame_count);
    int storeEntry(int stream_index, GifSharedStream::GifStreamData *data);
    void removeEntry(int stream_index);

    int registerKeyStream(int key, int hardware_key, GifSharedStream::GifStreamData *data);
    int registerBackgroundStream(GifSharedStream::GifStreamData *data, int x, int y, uint8_t fb_layer);

#ifdef WITH_ANIMATEDGIF
    int decodeGifToStream(const uint8_t *data, size_t length, int target_width, int target_height, GifSharedStream::GifStreamData *&out);
#endif
};
