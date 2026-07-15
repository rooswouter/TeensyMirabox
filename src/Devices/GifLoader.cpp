#include "MiraBox.h"
#include "GifLoader.h"

#include "GifConfig.h"
#include "GifController.h"
#include "StreamDock.h"

#include <Arduino.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef WITH_ANIMATEDGIF
#include <AnimatedGIF.h>
#include <JPEGENC.h>
#include <SD.h>
#endif

GifLoader::GifLoader(StreamDock &device, GifController &gif_controller)
    : device_(device), gif_controller_(gif_controller) {
        for (size_t i = 0; i < MAX_STREAMS; ++i) {
            entry_indices_[i] = -1;
        }
    }

void GifLoader::streamRemovedThunk(int stream_index, void *context) {
    if (context != nullptr) {
        static_cast<GifLoader *>(context)->onStreamRemoved(stream_index);
    }
}

void GifLoader::freeStreamData(GifSharedStream::GifStreamData *data) {
    if (data == nullptr) {
        return;
    }
    if (data->frames != nullptr) {
        for (size_t i = 0; i < data->frame_count; ++i) {
            free(data->frames[i]);
        }
        free(data->frames);
        data->frames = nullptr;
    }
    free(data->frame_sizes);
    free(data->delays);
    data->frame_sizes = nullptr;
    data->delays = nullptr;
    data->frame_count = 0;
    data->ref_count = 0;
    free(data);
}

void GifLoader::freePartialStreamData(GifSharedStream::GifStreamData *data, size_t decoded_frame_count) {
    if (data == nullptr) {
        return;
    }
    if (data->frames != nullptr) {
        for (size_t i = 0; i < decoded_frame_count; ++i) {
            free(data->frames[i]);
        }
        free(data->frames);
        data->frames = nullptr;
    }
    free(data->frame_sizes);
    free(data->delays);
    data->frame_sizes = nullptr;
    data->delays = nullptr;
    data->frame_count = 0;
    data->ref_count = 0;
    free(data);
}

void GifLoader::retainStreamData(GifSharedStream::GifStreamData *data) {
    if (data != nullptr) {
        ++data->ref_count;
    }
}

void GifLoader::releaseStreamData(GifSharedStream::GifStreamData *data) {
    if (data == nullptr) {
        return;
    }
    if (data->ref_count > 0) {
        --data->ref_count;
    }
    if (data->ref_count == 0) {
        freeStreamData(data);
    }
}

GifLoader::StreamEntry *GifLoader::findEntry(int stream_index) {
    for (size_t i = 0; i < MAX_STREAMS; ++i) {
        if (entry_indices_[i] == stream_index && entries_[i].active) {
            return &entries_[i];
        }
    }
    return nullptr;
}

const GifLoader::StreamEntry *GifLoader::findEntry(int stream_index) const {
    for (size_t i = 0; i < MAX_STREAMS; ++i) {
        if (entry_indices_[i] == stream_index && entries_[i].active) {
            return &entries_[i];
        }
    }
    return nullptr;
}

int GifLoader::storeEntry(int stream_index, GifSharedStream::GifStreamData *data) {
    removeEntry(stream_index);
    for (size_t i = 0; i < MAX_STREAMS; ++i) {
        if (!entries_[i].active) {
            entries_[i].data = data;
            entries_[i].active = true;
            entry_indices_[i] = stream_index;
            return 0;
        }
    }
    return -1;
}

void GifLoader::removeEntry(int stream_index) {
    if (StreamEntry *entry = findEntry(stream_index)) {
        releaseStreamData(entry->data);
        entry->data = nullptr;
        entry->active = false;
        for (size_t i = 0; i < MAX_STREAMS; ++i) {
            if (entry_indices_[i] == stream_index) {
                entry_indices_[i] = -1;
                break;
            }
        }
    }
}

void GifLoader::onStreamRemoved(int stream_index) {
    removeEntry(stream_index);
}

void GifLoader::releaseKey(int key) {
    int hardware_key = 0;
    if (key < 1) {
        return;
    }
    hardware_key = device_.get_image_key(static_cast<ButtonKey>(key));
    if (hardware_key >= 0) {
        removeEntry(hardware_key);
    }
}

void GifLoader::releaseBackground() {
    removeEntry(GifController::BACKGROUND_INDEX);
}

void GifLoader::releaseAll() {
    for (size_t i = 0; i < MAX_STREAMS; ++i) {
        if (entries_[i].active) {
            releaseStreamData(entries_[i].data);
            entries_[i].data = nullptr;
            entries_[i].active = false;
            entry_indices_[i] = -1;
        }
    }
}

GifSharedStream GifLoader::exportKeyStream(int key) const {
    GifSharedStream shared;
    if (key < 1) {
        return shared;
    }
    const int hardware_key = device_.get_image_key(static_cast<ButtonKey>(key));
    if (hardware_key < 0) {
        return shared;
    }
    if (const StreamEntry *entry = findEntry(hardware_key)) {
        shared.data_ = entry->data;
    }
    return shared;
}

GifSharedStream GifLoader::exportBackgroundStream() const {
    GifSharedStream shared;
    if (const StreamEntry *entry = findEntry(GifController::BACKGROUND_INDEX)) {
        shared.data_ = entry->data;
    }
    return shared;
}

#ifndef WITH_ANIMATEDGIF

int GifLoader::loadKeyGif(int, const uint8_t *, size_t) {
    return -1;
}

int GifLoader::loadBackgroundGif(const uint8_t *, size_t, int, int, uint8_t) {
    return -1;
}

int GifLoader::loadKeyGifFile(int, const char *) {
    return -1;
}

int GifLoader::loadBackgroundGifFile(const char *, int, int, uint8_t) {
    return -1;
}

int GifLoader::loadKeyGifShared(int, const GifSharedStream &) {
    return -1;
}

int GifLoader::loadBackgroundGifShared(const GifSharedStream &, int, int, uint8_t) {
    return -1;
}

#else

namespace {

struct GifRgbContext {
    uint8_t *rgb = nullptr;
    int width = 0;
    int height = 0;
};

void *gifAllocCallback(uint32_t size) {
    return malloc(size);
}

void gifFreeCallback(void *buffer) {
    free(buffer);
}

void releaseGifFrameBuf(AnimatedGIF &gif) {
    gif.freeFrameBuf(gifFreeCallback);
}

void gifDrawRgb888(GIFDRAW *draw) {
    auto *ctx = static_cast<GifRgbContext *>(draw->pUser);
    if (ctx == nullptr || ctx->rgb == nullptr || draw->y < 0 || draw->y >= ctx->height) {
        return;
    }

    const int dest_x = draw->iX;
    if (dest_x < 0 || dest_x >= ctx->width) {
        return;
    }

    const int copy_width = draw->iWidth;
    if (copy_width <= 0 || dest_x + copy_width > ctx->width) {
        return;
    }

    uint8_t *dest = ctx->rgb + (static_cast<size_t>(draw->y) * static_cast<size_t>(ctx->width) + static_cast<size_t>(dest_x)) * 3;
    memcpy(dest, draw->pPixels, static_cast<size_t>(copy_width) * 3);
}

int normalizeDelay(int delay_ms) {
    if (delay_ms <= 0) {
        return GifController::DEFAULT_DELAY_MS;
    }
    return delay_ms < 10 ? 10 : delay_ms;
}

// JPEGENC JPEGE_PIXEL_RGB888 expects BGR byte order despite the name.
void swapRgbToBgr(uint8_t *pixels, int width, int height) {
    const size_t count = static_cast<size_t>(width) * static_cast<size_t>(height);
    for (size_t i = 0; i < count; ++i) {
        uint8_t *p = pixels + i * 3;
        const uint8_t r = p[0];
        p[0] = p[2];
        p[2] = r;
    }
}

size_t encodeRgbToJpeg(const uint8_t *rgb, int width, int height, uint8_t **out_jpeg) {
    *out_jpeg = nullptr;

    // Working buffer: JPEG output is far smaller than RGB, but leave headroom during encode.
    size_t buffer_size = static_cast<size_t>(width) * static_cast<size_t>(height) / 2;
    if (buffer_size < 4096) {
        buffer_size = 4096;
    }

    uint8_t *jpeg_buffer = static_cast<uint8_t *>(malloc(buffer_size));
    if (jpeg_buffer == nullptr) {
        return 0;
    }

    JPEGENC jpeg;
    if (jpeg.open(jpeg_buffer, static_cast<int>(buffer_size)) != JPEGE_SUCCESS) {
        free(jpeg_buffer);
        return 0;
    }

    JPEGENCODE encode;
    if (jpeg.encodeBegin(&encode, width, height, JPEGE_PIXEL_RGB888, JPEGE_SUBSAMPLE_420, GIF_JPEG_QUALITY) != JPEGE_SUCCESS) {
        jpeg.close();
        free(jpeg_buffer);
        return 0;
    }

    uint8_t *encode_pixels = const_cast<uint8_t *>(rgb);
    swapRgbToBgr(encode_pixels, width, height);

    if (jpeg.addFrame(&encode, encode_pixels, width * 3) != JPEGE_SUCCESS) {
        jpeg.close();
        free(jpeg_buffer);
        return 0;
    }

    const int encoded_size = jpeg.close();
    if (encoded_size <= 0) {
        free(jpeg_buffer);
        return 0;
    }

    const size_t encoded_bytes = static_cast<size_t>(encoded_size);
    uint8_t *shrunk = static_cast<uint8_t *>(realloc(jpeg_buffer, encoded_bytes));
    if (shrunk != nullptr) {
        jpeg_buffer = shrunk;
    }

    *out_jpeg = jpeg_buffer;
    return encoded_bytes;
}

} // namespace

int GifLoader::decodeGifToStream(const uint8_t *data, size_t length, int target_width, int target_height, GifSharedStream::GifStreamData *&out) {
    out = nullptr;
    if (data == nullptr || length == 0) {
        return -1;
    }
    if (length > GIF_MAX_FILE_BYTES) {
        return -1;
    }

    GifSharedStream::GifStreamData *stream = static_cast<GifSharedStream::GifStreamData *>(
        calloc(1, sizeof(GifSharedStream::GifStreamData)));
    if (stream == nullptr) {
        return -1;
    }
    stream->ref_count = 1;

    AnimatedGIF gif;
    gif.begin(GIF_PALETTE_RGB888);

    int result = gif.open(const_cast<uint8_t *>(data), static_cast<int>(length), gifDrawRgb888);
    if (gif.getLastError() != GIF_SUCCESS) {
        Serial.printf("GIF open failed: error %i, length %i\n", gif.getLastError(), length);
        freeStreamData(stream);
        return -1;
    }
    const int canvas_width = gif.getCanvasWidth();
    const int canvas_height = gif.getCanvasHeight();
    if (canvas_width != target_width || canvas_height != target_height) {
        gif.close();
        Serial.printf("GIF canvas %dx%d does not match required %dx%d\n", canvas_width, canvas_height, target_width, target_height);
        freeStreamData(stream);
        return -1;
    }
    if (canvas_width > GIF_MAX_CANVAS_WIDTH || canvas_height > GIF_MAX_CANVAS_HEIGHT) {
        gif.close();
        freeStreamData(stream);
        return -1;
    }
    if (gif.setDrawType(GIF_DRAW_COOKED) != GIF_SUCCESS) {
        gif.close();
        freeStreamData(stream);
        return -1;
    }
    if (gif.allocFrameBuf(gifAllocCallback) != GIF_SUCCESS) {
        gif.close();
        freeStreamData(stream);
        return -1;
    }
    GIFINFO info;
    result = gif.getInfo(&info);
    if (gif.getInfo(&info) != 1 || info.iFrameCount <= 0) {
        releaseGifFrameBuf(gif);
        gif.close();
        freeStreamData(stream);
        return -1;
    }
    size_t frame_count = static_cast<size_t>(info.iFrameCount);
    if (frame_count > GIF_MAX_FRAMES) {
        releaseGifFrameBuf(gif);
        gif.close();
        freeStreamData(stream);
        return -1;
    }
    stream->frames = static_cast<uint8_t **>(calloc(frame_count, sizeof(uint8_t *)));
    stream->frame_sizes = static_cast<size_t *>(calloc(frame_count, sizeof(size_t)));
    stream->delays = static_cast<int *>(calloc(frame_count, sizeof(int)));
    if (stream->frames == nullptr || stream->frame_sizes == nullptr || stream->delays == nullptr) {
        freePartialStreamData(stream, 0);
        releaseGifFrameBuf(gif);
        gif.close();
        return -1;
    }
    const size_t rgb_size = static_cast<size_t>(canvas_width) * static_cast<size_t>(canvas_height) * 3;
    uint8_t *rgb = static_cast<uint8_t *>(malloc(rgb_size));
    if (rgb == nullptr) {
        freePartialStreamData(stream, 0);
        releaseGifFrameBuf(gif);
        gif.close();
        return -1;
    }
    GifRgbContext rgb_ctx;
    rgb_ctx.rgb = rgb;
    rgb_ctx.width = canvas_width;
    rgb_ctx.height = canvas_height;

    gif.reset();
    for (size_t frame_index = 0; frame_index < frame_count; ++frame_index) {
        memset(rgb, 0, rgb_size);
        int delay_ms = GifController::DEFAULT_DELAY_MS;
        const int play_result = gif.playFrame(true, &delay_ms, &rgb_ctx);
        if (play_result < 0) {
            free(rgb);
            freePartialStreamData(stream, frame_index);
            releaseGifFrameBuf(gif);
            gif.close();
            return -1;
        }

        uint8_t *jpeg_data = nullptr;
        const size_t jpeg_size = encodeRgbToJpeg(rgb, canvas_width, canvas_height, &jpeg_data);
        if (jpeg_size == 0 || jpeg_data == nullptr) {
            free(rgb);
            freePartialStreamData(stream, frame_index);
            releaseGifFrameBuf(gif);
            gif.close();
            return -1;
        }

        stream->frames[frame_index] = jpeg_data;
        stream->frame_sizes[frame_index] = jpeg_size;
        stream->delays[frame_index] = normalizeDelay(delay_ms);
    }
    free(rgb);
    releaseGifFrameBuf(gif);
    gif.close();

    stream->frame_count = frame_count;
    out = stream;
    return 0;
}

int GifLoader::registerKeyStream(int key, int hardware_key, GifSharedStream::GifStreamData *data) {
    if (data == nullptr || data->frame_count == 0) {
        return -1;
    }

    if (gif_controller_.set_key_gif_stream(
            const_cast<const uint8_t *const *>(data->frames),
            data->frame_sizes,
            data->delays,
            data->frame_count,
            key) != 0) {
        return -1;
    }

    const uint8_t *frame0 = data->frames[0];
    const size_t frame0_size = data->frame_sizes[0];

    if (storeEntry(hardware_key, data) != 0) {
        gif_controller_.clear_key_gif(key);
        return -1;
    }

    device_.transport.setKeyImageStream(frame0, frame0_size, hardware_key);
    device_.refresh();
    device_.start_gif_loop();
    return 0;
}

int GifLoader::registerBackgroundStream(GifSharedStream::GifStreamData *data, int x, int y, uint8_t fb_layer) {
    if (data == nullptr || data->frame_count == 0) {
        return -1;
    }

    const ImageFormat format = device_.touchscreen_image_format();

    if (gif_controller_.set_background_gif_stream(
            const_cast<const uint8_t *const *>(data->frames),
            data->frame_sizes,
            data->delays,
            data->frame_count,
            x,
            y,
            fb_layer) != 0) {
        return -1;
    }

    const uint8_t *frame0 = data->frames[0];
    const size_t frame0_size = data->frame_sizes[0];

    if (storeEntry(GifController::BACKGROUND_INDEX, data) != 0) {
        gif_controller_.clear_background_gif();
        return -1;
    }

    device_.transport.setBackgroundFrameStream(
        frame0,
        frame0_size,
        format.width,
        format.height,
        x,
        y,
        fb_layer);
    device_.refresh();
    device_.start_gif_loop();
    return 0;
}

int GifLoader::loadKeyGif(int key, const uint8_t *data, size_t length) {
    if (!device_.feature_option.supportKeyGif || key < 1 || key > device_.image_keys()) {
        return -1;
    }
    
    const ImageFormat format = device_.key_image_format();
    if (format.format != ImageFileFormat::JPEG || format.width == 0 || format.height == 0) {
        return -1;
    }
    
    int hardware_key = device_.get_image_key(static_cast<ButtonKey>(key));
    if (hardware_key < 0) {
        return -1;
    }
    
    GifSharedStream::GifStreamData *stream = nullptr;
    if (decodeGifToStream(data, length, format.width, format.height, stream) != 0) {
        return -1;
    }

    const int result = registerKeyStream(key, hardware_key, stream);
    if (result != 0) {
        releaseStreamData(stream);
    }
    return result;
}

int GifLoader::loadKeyGifShared(int key, const GifSharedStream &shared) {
    if (!shared.valid() || !device_.feature_option.supportKeyGif || key < 1 || key > device_.image_keys()) {
        return -1;
    }

    const ImageFormat format = device_.key_image_format();
    if (format.format != ImageFileFormat::JPEG || format.width == 0 || format.height == 0) {
        return -1;
    }

    const int hardware_key = device_.get_image_key(static_cast<ButtonKey>(key));
    if (hardware_key < 0) {
        return -1;
    }

    retainStreamData(shared.data_);
    const int result = registerKeyStream(key, hardware_key, shared.data_);
    if (result != 0) {
        releaseStreamData(shared.data_);
    }
    return result;
}

int GifLoader::loadBackgroundGif(const uint8_t *data, size_t length, int x, int y, uint8_t fb_layer) {
    if (!device_.feature_option.supportBackgroundGif) {
        return -1;
    }

    const ImageFormat format = device_.touchscreen_image_format();
    if (format.width == 0 || format.height == 0) {
        return -1;
    }

    GifSharedStream::GifStreamData *stream = nullptr;
    if (decodeGifToStream(data, length, format.width, format.height, stream) != 0) {
        return -1;
    }

    const int result = registerBackgroundStream(stream, x, y, fb_layer);
    if (result != 0) {
        releaseStreamData(stream);
    }
    return result;
}

int GifLoader::loadBackgroundGifShared(const GifSharedStream &shared, int x, int y, uint8_t fb_layer) {
    if (!shared.valid() || !device_.feature_option.supportBackgroundGif) {
        return -1;
    }

    const ImageFormat format = device_.touchscreen_image_format();
    if (format.width == 0 || format.height == 0) {
        return -1;
    }

    retainStreamData(shared.data_);
    const int result = registerBackgroundStream(shared.data_, x, y, fb_layer);
    if (result != 0) {
        releaseStreamData(shared.data_);
    }
    return result;
}

int GifLoader::loadKeyGifFile(int key, const char *sd_path) {
    if (sd_path == nullptr || sd_path[0] == '\0') {
        return -1;
    }
    
    File file = SD.open(sd_path, FILE_READ);
    if (!file) {
        Serial.printf("GIF file %s not found\n", sd_path);
        return -1;
    }

    const size_t file_size = file.size();
    if (file_size == 0 || file_size > GIF_MAX_FILE_BYTES) {
        file.close();
        return -1;
    }

    uint8_t *buffer = static_cast<uint8_t *>(malloc(file_size));
    if (buffer == nullptr) {
        file.close();
        return -1;
    }

    const size_t read_size = file.read(buffer, file_size);
    file.close();
    if (read_size != file_size) {
        free(buffer);
        return -1;
    }

    const int result = loadKeyGif(key, buffer, read_size);
    free(buffer);
    return result;
}

int GifLoader::loadBackgroundGifFile(const char *sd_path, int x, int y, uint8_t fb_layer) {
    if (sd_path == nullptr || sd_path[0] == '\0') {
        return -1;
    }

    File file = SD.open(sd_path, FILE_READ);
    if (!file) {
        Serial.printf("GIF file %s not found\n", sd_path);
        return -1;
    }

    const size_t file_size = file.size();
    if (file_size == 0 || file_size > GIF_MAX_FILE_BYTES) {
        file.close();
        return -1;
    }

    uint8_t *buffer = static_cast<uint8_t *>(malloc(file_size));
    if (buffer == nullptr) {
        file.close();
        return -1;
    }

    const size_t read_size = file.read(buffer, file_size);
    file.close();
    if (read_size != file_size) {
        free(buffer);
        return -1;
    }

    const int result = loadBackgroundGif(buffer, read_size, x, y, fb_layer);
    free(buffer);
    return result;
}

#endif
