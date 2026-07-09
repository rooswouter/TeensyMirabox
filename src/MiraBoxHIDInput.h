#ifndef __MiraBoxHIDInput_h_
#define __MiraBoxHIDInput_h_
#include <Arduino.h>
#include <USBHost_t36.h>

void indent_level(int level);
void dump_hexbytes(const void *ptr, uint32_t len, uint32_t indent);

class MiraBoxHIDInput : public USBHIDInput {
public:
    MiraBoxHIDInput(USBHost &host, uint32_t index = 0, uint32_t usage = 0)
        : host_(host), index_(index), fixed_usage_(usage) {
        init();
    }

    uint32_t usage(void) { return usage_; }
    static bool show_raw_data;
    static bool show_formated_data;
    static bool changed_data_only;

    bool isConnected() const { return mydevice != nullptr && driver_ != nullptr; }
    bool sendPacket(const uint8_t *buffer, int cb = -1, unsigned long timeout_ms = 10000);
    // HID GET_REPORT(Input) control transfer; desktop SDKs do this during init
    // and some StreamDock firmware needs it before it starts sending events.
    bool requestInputReport(uint8_t report_id, uint16_t length);
    void pumpHost() { host_.Task(); }
    uint16_t inputReportSize() const;
    uint16_t outputReportSize() const;

    bool hasReadData() const;
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
};
#endif // __MiraBoxHIDInput_h_
