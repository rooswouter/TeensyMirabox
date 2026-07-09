#include <USBHost_t36.h>
#include "DeviceManager.h"

USBHost myusb;
DeviceManager dm(myusb);

const char *images[] = { "85x85/key1.jpg", "85x85/key2.jpg", "85x85/key3.jpg", "85x85/key4.jpg", "85x85/key5.jpg", "85x85/key6.jpg"};
void onAdded(StreamDock* device) {
  Serial.println("Device added");
  printf("ID: %s\n", device->id().c_str());
  printf("Serial: %s\n", device->get_serial_number().c_str());
  printf("Path: %s\n", device->getPath().c_str());
  
  printf("device->get_device_type() = %d\n", (int)device->get_device_type());
  if(device->get_device_type() == device_type::k1pro) {
    Serial.println("K1Pro setting to SDK Mode");
    K1Pro *k1Pro = (K1Pro*)(device);
    k1Pro->keyboard_mode(1);
  }

  device->clearAllIcon();
  // image_keys() can exceed the images[] array size (293S has 18 keys),
  // so clamp the loop to avoid reading past the end of the array.
  const int image_count = (int)(sizeof(images) / sizeof(images[0]));
  const int keys_to_set = min(device->image_keys(), image_count);
  for (int i = 0; i < keys_to_set; i++) {
    device->set_key_image(i+1, images[i]);
  }
  device->set_key_callback(key_callback);
  device->refresh();

}

void onRemoved(StreamDock* d) {
  Serial.println("Device removed");
}

void key_callback(StreamDock *device, const InputEvent &event)
{
  switch(event.event_type) {
    case EventType::BUTTON:
      Serial.printf("Key %d %s\n", (int)event.key, event.state == 1 ? "Pressed" : "Released" );
      break;
    case EventType::KNOB_ROTATE:
      Serial.printf("Knob %d rotated %s\n", (int)event.knob_id, (int)event.direction == 0 ? "Left" : "Right" );
      break;
    case EventType::KNOB_PRESS:
      Serial.printf("Knob %d pressed %s\n", (int)event.knob_id, event.state == 1 ? "Pressed" : "Released" );
      break;

    default:
      break;

  }
}

void setup() {
  Serial.begin(115200);
  dm.setDeviceChangeCallback(onAdded, onRemoved);
  dm.begin(true, true);
  MiraBoxHIDInput::show_raw_data = true;
  myusb.begin();  // required
}
void loop() {
  dm.poll();      // calls host_.Task() internally
}