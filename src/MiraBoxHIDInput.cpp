#include "MiraBoxHIDInput.h"
bool MiraBoxHIDInput::show_raw_data = false;
bool MiraBoxHIDInput::show_formated_data = false;
bool MiraBoxHIDInput::changed_data_only = false;

void MiraBoxHIDInput::init() {
  USBHost::contribute_Transfers(mytransfers, sizeof(mytransfers) / sizeof(Transfer_t));
  USBHIDParser::driver_ready_for_hid_collection(this);
}

hidclaim_t MiraBoxHIDInput::claim_collection(USBHIDParser *driver, Device_t *dev, uint32_t topusage) {
  printf("MiraBoxHIDInput::claim_collection(driver: %p, dev: %p, topusage: %x)\n", driver, dev, topusage);
  if (show_raw_data) {
    Serial.printf("MiraBoxHIDInput(%u : %p : %p) Claim: %x:%x usage: %x", index_, this, driver, dev->idVendor, dev->idProduct, topusage);
    Serial.printf(" SubClass: %x Protocol: %x",  driver->interfaceSubClass(), driver->interfaceProtocol());
  }

  const uint16_t usage_page = (uint16_t)(topusage >> 16);
  const uint16_t usage = (uint16_t)(topusage & 0xffff);
  if (mydevice != NULL && dev != mydevice) {
    if (show_raw_data) Serial.println("- NO (Device)");
    return CLAIM_NO;
  }

  /* 
    For 293S, N3   
    if (usage_page != 0xFFA0 || index_ != 2) {
    For K1Pro
    if (usage_page != 0x000C || index_ != 2) {
  */
  if (!(usage_page == 0x000C || usage_page == 0xFFA0) || index_ != 2) {
    if (show_raw_data) Serial.println(" - NO (not vendor-defined usage page)");
    return CLAIM_NO;
  }
  tmp_index_++;
  bool dump_hid_info = (usage_ == 0);


  mydevice = dev;
  collections_claimed++;
  usage_ = topusage;
  driver_ = driver;  // remember the driver.
  if (show_raw_data) Serial.println(" - Yes");

  // Required for K1PRo, N3, always used ever for 513 bytes devices?
  if (index_ == 2) {
    static uint8_t rx_buf1[1024] __attribute__((aligned(32)));
    static uint8_t rx_buf2[1024] __attribute__((aligned(32)));
    static uint8_t tx_buf1[1024] __attribute__((aligned(32)));
    static uint8_t tx_buf2[1024] __attribute__((aligned(32)));
    driver->setRXBuffers(rx_buf1, rx_buf2, 0);
    driver->setTXBuffers(tx_buf1, tx_buf2, 0);

    // Windows/Linux HID class drivers send SET_IDLE to every HID interface
    // during enumeration. Some StreamDock firmware waits for it before it
    // starts sending input reports, so mimic the OS behavior here.
    driver->sendControlPacket(0x21, 10, 0, driver->interfaceNumber(), 0, nullptr);
  }

  // if Boot Mouse - then set idle
  if ((driver->interfaceSubClass() == 1) && (driver->interfaceProtocol() == 1)) {
    if (show_raw_data) {
      USBHDBGSerial.printf(">> Boot Keyboard - Send SET_IDLE <<\n");
    }
    driver->sendControlPacket(0x21, 10, 0, 0, 0, nullptr); //10=SET_IDLE

  }

  // Lets try to dump the whole HID Report descriptor only the first time
  if (dump_hid_info && show_raw_data) dumpHIDReportDescriptor(driver);
  
  return CLAIM_INTERFACE;  // We want
}

void MiraBoxHIDInput::disconnect_collection(Device_t *dev) {
  if (--collections_claimed == 0) {
    mydevice = NULL;
    usage_ = 0;
    driver_ = nullptr;
    read_head_ = 0;
    read_tail_ = 0;
  }
}

bool MiraBoxHIDInput::sendPacket(const uint8_t *buffer, int cb, unsigned long timeout_ms) {
  if (!driver_) {
    return false;
  }

  const int out_size = (int)driver_->outSize();
  if (out_size <= 0) {
    return false;
  }

  // Our external TX buffers (see claim_collection) are 1024 bytes; transfers
  // larger than the endpoint size are split into packets by the host driver.
  const int max_len = (index_ == 2) ? 1024 : out_size;
  if (cb < 0) {
    cb = out_size;
  } else if (cb > max_len) {
    cb = max_len;
  }

  const unsigned long deadline = millis() + timeout_ms;
  while ((long)(millis() - deadline) < 0) {
    if (driver_->sendPacket(buffer, cb)) {
      return true;
    }
    host_.Task();
    yield();
  }
  return false;
}

bool MiraBoxHIDInput::requestInputReport(uint8_t report_id, uint16_t length) {
  if (!driver_) {
    return false;
  }
  static uint8_t report_buf[1024] __attribute__((aligned(32)));
  if (length > sizeof(report_buf)) {
    length = sizeof(report_buf);
  }
  // GET_REPORT: bmRequestType 0xA1, bRequest 0x01, wValue = (Input << 8) | report_id
  return driver_->sendControlPacket(0xA1, 0x01, 0x0100 | report_id, driver_->interfaceNumber(), length, report_buf);
}

uint16_t MiraBoxHIDInput::inputReportSize() const {
  return driver_ ? driver_->inSize() : 0;
}

uint16_t MiraBoxHIDInput::outputReportSize() const {
  return driver_ ? driver_->outSize() : 0;
}

bool MiraBoxHIDInput::hasReadData() const {
  return read_head_ != read_tail_;
}

size_t MiraBoxHIDInput::read(uint8_t *buffer, size_t capacity) {
  if (!hasReadData() || buffer == nullptr || capacity == 0) {
    return 0;
  }

  const uint8_t slot = read_tail_;
  const size_t length = min((size_t)read_lengths_[slot], capacity);
  memcpy(buffer, read_queue_[slot], length);
  read_tail_ = (read_tail_ + 1) % READ_QUEUE_SLOTS;
  return length;
}

void MiraBoxHIDInput::queueInputReport(const uint8_t *data, uint32_t length) {
  if (data == nullptr || length == 0) {
    return;
  }

  const uint8_t next_head = (read_head_ + 1) % READ_QUEUE_SLOTS;
  if (next_head == read_tail_) {
    read_tail_ = (read_tail_ + 1) % READ_QUEUE_SLOTS;
  }

  const size_t copy_len = min((size_t)length, (size_t)MAX_READ_REPORT_SIZE);
  memcpy(read_queue_[read_head_], data, copy_len);
  read_lengths_[read_head_] = copy_len;
  read_head_ = next_head;
}


bool MiraBoxHIDInput::hid_process_in_data(const Transfer_t *transfer) {
  // return true if we are not showing formated data...
  hid_input_begin_level_ = 0;     // always make sure we reset to 0
  count_usages_ = index_usages_;  // remember how many we output for this one
  index_usages_ = 0;              // reset the index back to zero
  printf("hid_process_in_data() transfer->length: %d\n", transfer->length);
  // Index 1 is keyboard, index 2 is mirabox buttons
  switch (index_) {
    case 1:
      handle_keyboard_data(transfer);
      return !show_formated_data;
    case 2:
      queueInputReport((const uint8_t *)transfer->buffer, transfer->length);
      handle_mirabox_buttons_data(transfer);
      if (show_raw_data) {
        Serial.printf("HID(%u : %x) IN: ", index_, usage_);
        dump_hexbytes(transfer->buffer, min(transfer->length, (uint32_t)32), 0);
      }
      // Always bypass HID parse(); StreamDock protocol needs raw report bytes.
      return true;
    default:
      break;
  }
  return !show_formated_data;
}


void MiraBoxHIDInput::handle_keyboard_data(const Transfer_t *transfer)
{
    if (show_raw_data) {
        Serial.print("Keyboard: ");
        dump_hexbytes(transfer->buffer, transfer->length, 16);
    }
}

void MiraBoxHIDInput::handle_mirabox_buttons_data(const Transfer_t *transfer)
{
    (void)transfer;
    // Input is queued in hid_process_in_data and parsed by StreamDock::poll().
}

bool MiraBoxHIDInput::hid_process_out_data(const Transfer_t *transfer) {
  if (show_raw_data) {
    Serial.printf("MiraBoxHIDInput::hid_process_out_data: %x\n", usage_);
  }
  (void)transfer;
  return true;
}


void MiraBoxHIDInput::hid_input_begin(uint32_t topusage, uint32_t type, int lgmin, int lgmax) {
  if (changed_data_only || !show_formated_data) return;

  indent_level(hid_input_begin_level_);
  Serial.printf("Begin topusage:%x type:%x min:%d max:%d\n", topusage, type, lgmin, lgmax);
  if (hid_input_begin_level_ < 2)
    hid_input_begin_level_++;
}

void MiraBoxHIDInput::hid_input_data(uint32_t usage, int32_t value) {
  if (!show_formated_data) return;

  bool output_data = !changed_data_only;

  // See if something changed.
  if (index_usages_ < count_usages_) {
    if ((usage != usages_[index_usages_]) || (value != values_[index_usages_])) {
      output_data = true;
    }
  } else {
    output_data = true;
  }
  if (index_usages_ < MAX_CHANGE_TRACKED) {
    usages_[index_usages_] = usage;
    values_[index_usages_] = value;
    index_usages_++;
  }

  if (output_data) {
    indent_level(hid_input_begin_level_);
    Serial.printf("usage=%X, value=%d ", usage, value);
    if ((value >= ' ') && (value <= '~')) Serial.printf(":%c", value);

    // maybe print out some information about some of the Usage numbers that we know about
    // The information comes from the USB document, HID Usage Tables
    // https://www.usb.org/sites/default/files/documents/hut1_12v2.pdf

    uint16_t usage_page = usage >> 16;
    usage = usage & 0xffff;  // keep the lower part
    //printUsageInfo(usage_page, usage);

    Serial.println();
  }
}



void MiraBoxHIDInput::hid_input_end() {
  if (changed_data_only || !show_formated_data) return;
  hid_input_begin_level_--;
  indent_level(hid_input_begin_level_);
  Serial.println("END:");
}

void MiraBoxHIDInput::dumpHIDReportDescriptor(USBHIDParser *phidp) {
  const uint8_t *p = phidp->getHIDReportDescriptor();
  uint16_t report_size = phidp->getHIDReportDescriptorSize();
  const uint8_t *pend = p + report_size;
  uint8_t collection_level = 0;
  uint16_t usage_page = 0;
  enum { USAGE_LIST_LEN = 24 };
  uint16_t usage[USAGE_LIST_LEN] = { 0, 0 };
  uint8_t usage_count = 0;
  uint32_t topusage;
  cnt_feature_reports_ = 0;
  uint8_t last_report_id = 0;
  Serial.printf("\nHID Report Descriptor (%p) size: %u\n", p, report_size);
  while (p < pend) {
    uint8_t tag = *p;
    for (uint8_t i = 0; i < collection_level; i++) Serial.print("  ");
    Serial.printf("  %02X", tag);

    if (tag == 0xFE) {  // Long Item (unsupported)
      p += p[1] + 3;
      continue;
    }
    uint32_t val;
    switch (tag & 0x03) {  // Short Item data
      case 0:
        val = 0;
        p++;
        break;
      case 1:
        val = p[1];
        // could be better;
        Serial.printf(" %02X", p[1]);
        p += 2;
        break;
      case 2:
        val = p[1] | (p[2] << 8);
        Serial.printf(" %02X %02X", p[1], p[2]);
        p += 3;
        break;
      case 3:
        val = p[1] | (p[2] << 8) | (p[3] << 16) | (p[4] << 24);
        Serial.printf(" %02X %02X %02X %02X", p[1], p[2], p[3], p[4]);
        p += 5;
        break;
    }
    if (p > pend) break;

    bool reset_local = false;
    switch (tag & 0xfc) {
      case 0x4:  //usage Page
        {
          usage_page = val;
          Serial.printf("\t// Usage Page(%x) - ", val);
          switch (usage_page) {
            case 0x01: Serial.print("Generic Desktop"); break;
            case 0x06: Serial.print("Generic Device Controls"); break;
            case 0x07: Serial.print("Keycode"); break;
            case 0x08: Serial.print("LEDs"); break;
            case 0x09: Serial.print("Button"); break;
            case 0x0C: Serial.print("Consumer"); break;
            case 0x0D:
            case 0xFF0D: Serial.print("Digitizer"); break;
            default: 
              if (usage_page >= 0xFF00) Serial.print("Vendor Defined");
              else Serial.print("Other ?"); 
              break;
          }
        }
        break;
      case 0x08:  //usage
        Serial.printf("\t// Usage(%x) -", val);
        //printUsageInfo(usage_page, val);
        if (usage_count < USAGE_LIST_LEN) {
          // Usages: 0 is reserved 0x1-0x1f is sort of reserved for top level things like
          // 0x1 - Pointer - A collection... So lets try ignoring these
          if (val > 0x1f) {
            usage[usage_count++] = val;
          }
        }
        break;
      case 0x14:  // Logical Minimum (global)
        Serial.printf("\t// Logical Minimum(%x)", val);
        break;
      case 0x24:  // Logical Maximum (global)
        Serial.printf("\t// Logical maximum(%x)", val);
        break;
      case 0x74:  // Report Size (global)
        Serial.printf("\t// Report Size(%x)", val);
        break;
      case 0x94:  // Report Count (global)
        Serial.printf("\t// Report Count(%x)", val);
        break;
      case 0x84:  // Report ID (global)
        Serial.printf("\t// Report ID(%x)", val);
        last_report_id = val;
        break;
      case 0x18:  // Usage Minimum (local)
        usage[0] = val;
        usage_count = 255;
        Serial.printf("\t// Usage Minimum(%x) - ", val);
        //printUsageInfo(usage_page, val);
        break;
      case 0x28:  // Usage Maximum (local)
        usage[1] = val;
        usage_count = 255;
        Serial.printf("\t// Usage Maximum(%x) - ", val);
        //printUsageInfo(usage_page, val);
        break;
      case 0xA0:  // Collection
        Serial.printf("\t// Collection(%x)", val);
        // discard collection info if not top level, hopefully that's ok?
        if (collection_level == 0) {
          topusage = ((uint32_t)usage_page << 16) | usage[0];
          Serial.printf(" top Usage(%x)", topusage);
          collection_level++;
        }
        reset_local = true;
        break;
      case 0xC0:  // End Collection
        Serial.print("\t// End Collection");
        if (collection_level > 0) collection_level--;
        break;

      case 0x80:  // Input
        Serial.printf("\t// Input(%x)\t// (", val);
        print_input_output_feature_bits(val);
        reset_local = true;
        break;
      case 0x90:  // Output
        Serial.printf("\t// Output(%x)\t// (", val);
        print_input_output_feature_bits(val);
        reset_local = true;
        break;
      case 0xB0:  // Feature
        Serial.printf("\t// Feature(%x)\t// (", val);
        print_input_output_feature_bits(val);
        if (cnt_feature_reports_ < MAX_FEATURE_REPORTS) {
          feature_report_ids_[cnt_feature_reports_++] = last_report_id;
        }
        reset_local = true;
        break;

      case 0x34:  // Physical Minimum (global)
        Serial.printf("\t// Physical Minimum(%x)", val);
        break;
      case 0x44:  // Physical Maximum (global)
        Serial.printf("\t// Physical Maximum(%x)", val);
        break;
      case 0x54:  // Unit Exponent (global)
        Serial.printf("\t// Unit Exponent(%x)", val);
        break;
      case 0x64:  // Unit (global)
        Serial.printf("\t// Unit(%x)", val);
        break;
    }
    if (reset_local) {
      usage_count = 0;
      usage[0] = 0;
      usage[1] = 0;
    }

    Serial.println();
  }
}

void MiraBoxHIDInput::print_input_output_feature_bits(uint8_t val) {
  Serial.print((val & 0x01)? "Constant" : "Data");  
  Serial.print((val & 0x02)? ", Variable" : ", Array");  
  Serial.print((val & 0x04)? ", Relative" : ", Absolute");  
  if (val & 0x08) Serial.print(", Wrap");
  if (val & 0x10) Serial.print(", Non Linear");
  if (val & 0x20) Serial.print(", No Preferred");
  if (val & 0x40) Serial.print(", Null State");
  if (val & 0x80) Serial.print(", Volatile");
  if (val & 0x100) Serial.print(", Buffered Bytes");
  Serial.print(")");  
}

bool MiraBoxHIDInput::is_input_device_config_packet(const Transfer_t *transfer)
{
    const uint8_t *buffer = (const uint8_t *)transfer->buffer;
    uint32_t length = transfer->length;
    // Checks if the packet is a configuration packet, as send by the K1Pro when in native keyboard mode
    // message is \x04DEVCFG
    return (
        length >= 12
        && buffer[0] == 0x04
        && buffer[1] == 0x44
        && buffer[2] == 0x45
        && buffer[3] == 0x56
        && buffer[4] == 0x43
        && buffer[5] == 0x46
        && buffer[6] == 0x47
    );
}

bool MiraBoxHIDInput::is_input_event_packet(const Transfer_t *transfer)
{
    const uint8_t *buffer = (const uint8_t *)transfer->buffer;
    uint32_t length = transfer->length;
    // Assume we are a K1 Pro for now
    
    return (
        length >= 12
        && buffer[0] == 0x04
        && buffer[1] == 0x41
        && buffer[2] == 0x43
        && buffer[3] == 0x4B
        && buffer[6] == 0x4F
        && buffer[7] == 0x4B
    );
        /* Other devices
    return (
        len(data) >= 11
        and data[0] == 0x41
        and data[1] == 0x43
        and data[2] == 0x4B
        and data[5] == 0x4F
        and data[6] == 0x4B
    )
    */
}


void dump_hexbytes(const void *ptr, uint32_t len, uint32_t indent) {
  if (!MiraBoxHIDInput::show_raw_data && !MiraBoxHIDInput::show_formated_data) return;
  if (ptr == NULL || len == 0) return;
  uint32_t count = 0;
  //  if (len > 64) len = 64; // don't go off deep end...
  const uint8_t *p = (const uint8_t *)ptr;
  while (len--) {
    if (*p < 16) Serial.print('0');
    Serial.print(*p++, HEX);
    count++;
    if (((count & 0x1f) == 0) && len) {
      Serial.print("\n");
      for (uint32_t i = 0; i < indent; i++) Serial.print(" ");
    } else
      Serial.print(' ');
  }
  Serial.println();
}

void indent_level(int level) {
  if ((level > 5) || (level < 0)) return;  // bail if something is off...
  while (level--) Serial.print("  ");
}