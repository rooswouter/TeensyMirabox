#ifndef __MiraBoxHIDInput_h_
#define __MiraBoxHIDInput_h_
#include <Arduino.h>
#include <USBHost_t36.h>

/** @brief Indent helper for formatted HID debug output. */
void indent_level(int level);

/** @brief Print a hex dump to Serial (gated by debug flags). */
void dump_hexbytes(const void *ptr, uint32_t len, uint32_t indent);

/**
 * @file MiraBoxHIDInput.h
 * @brief USBHost_t36 HID driver for StreamDock control and keyboard interfaces.
 *
 * Two instances are typically used per device slot:
 * - `index` 1 — boot keyboard interface
 * - `index` 2 — vendor control interface (images, keys, SDK protocol)
 *
 * Interrupt-IN reports for the control interface are queued and read via `read()`.
 */

/**
 * @class MiraBoxHIDInput
 * @brief Claims StreamDock HID collections and bridges USBHost_t36 to LibUSBHIDAPI.
 */
class MiraBoxHIDInput : public USBHIDInput {
public:
    /**
     * @brief Construct and register with the USB host HID driver list.
     * @param host USBHost reference from the sketch.
     * @param index 1 = boot keyboard, 2 = StreamDock control (default 0).
     * @param usage Optional fixed top-level usage to claim (0 = any).
     */
    MiraBoxHIDInput(USBHost &host, uint32_t index = 0, uint32_t usage = 0)
        : host_(host), index_(index), fixed_usage_(usage) {
        init();
    }

    /** @return Claimed top-level HID usage, or 0 if not connected. */
    uint32_t usage(void) { return usage_; }

    /** When true, print raw HID claim and report hex dumps to Serial. */
    static bool show_raw_data;

    /** When true, print parsed HID usage fields to Serial. */
    static bool show_formated_data;

    /** When true, only print HID fields that changed since the last report. */
    static bool changed_data_only;

    /**
     * @brief True if this driver has claimed a device collection.
     * @return `true` when a USB device and HID parser are bound.
     */
    bool isConnected() const { return mydevice != nullptr && driver_ != nullptr; }

    /**
     * @brief Send an output report on the claimed HID interface.
     * @param buffer Report bytes (padded to endpoint size by the caller or transport).
     * @param cb Byte count, or -1 to use the endpoint report size.
     * @param timeout_ms Maximum time to wait for the OUT pipe (default 10000 ms).
     * @return `true` if the packet was queued successfully.
     */
    bool sendPacket(const uint8_t *buffer, int cb = -1, unsigned long timeout_ms = 10000);

    /**
     * @brief HID GET_REPORT (Input) control transfer.
     *
     * Mirrors desktop `hid_get_input_report` used during init on some firmware.
     *
     * @param report_id Report ID byte (0x00 or 0x04 for K1 Pro).
     * @param length Buffer length to request.
     * @return `true` if the control transfer was queued.
     */
    bool requestInputReport(uint8_t report_id, uint16_t length);

    /** @brief Run one USB host task iteration (call during blocking writes). */
    void pumpHost() { host_.Task(); }

    /** @return Interrupt-IN endpoint report size from the HID parser. */
    uint16_t inputReportSize() const;

    /** @return Interrupt-OUT endpoint report size from the HID parser. */
    uint16_t outputReportSize() const;

    /**
     * @brief True if one or more input reports are waiting in the queue.
     */
    bool hasReadData() const;

    /**
     * @brief Dequeue one input report (non-blocking).
     * @param buffer Destination buffer.
     * @param capacity Maximum bytes to copy.
     * @return Number of bytes copied, or 0 if the queue is empty.
     */
    size_t read(uint8_t *buffer, size_t capacity);

protected:
    virtual hidclaim_t claim_collection(USBHIDParser *driver, Device_t *dev, uint32_t topusage);
    virtual bool hid_process_in_data(const Transfer_t *transfer);
    virtual bool hid_process_out_data(const Transfer_t *transfer);
    virtual void hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax);
    virtual void hid_input_data(uint32_t usage, int32_t value);
    virtual void hid_input_end();
    virtual void disconnect_collection(Device_t *dev);

private:
    void init();
    void queueInputReport(const uint8_t *data, uint32_t length);

    void dumpHIDReportDescriptor(USBHIDParser *phidp);
    void print_input_output_feature_bits(uint8_t val);

    void handle_keyboard_data(const Transfer_t *transfer);
    void handle_mirabox_buttons_data(const Transfer_t *transfer);

    bool is_input_device_config_packet(const Transfer_t *transfer);
    bool is_input_event_packet(const Transfer_t *transfer);

    USBHIDParser *driver_ = nullptr;
    USBHost &host_;
    uint8_t collections_claimed = 0;
    volatile int hid_input_begin_level_ = 0;
    uint32_t index_;
    uint32_t fixed_usage_;

    uint32_t usage_ = 0;
    const static int MAX_CHANGE_TRACKED = 512;
    uint32_t usages_[MAX_CHANGE_TRACKED];
    int32_t values_[MAX_CHANGE_TRACKED];
    int count_usages_ = 0;
    int index_usages_ = 0;

    enum { MAX_FEATURE_REPORTS = 20 };
    uint8_t feature_report_ids_[MAX_FEATURE_REPORTS];
    uint8_t cnt_feature_reports_ = 0;

    enum {
        READ_QUEUE_SLOTS = 8,
        MAX_READ_REPORT_SIZE = 1024,
    };
    uint8_t read_queue_[READ_QUEUE_SLOTS][MAX_READ_REPORT_SIZE];
    uint16_t read_lengths_[READ_QUEUE_SLOTS];
    volatile uint8_t read_head_ = 0;
    volatile uint8_t read_tail_ = 0;

    Transfer_t mytransfers[2] __attribute__((aligned(32)));

    int tmp_index_ = 0;
};
#endif // __MiraBoxHIDInput_h_
