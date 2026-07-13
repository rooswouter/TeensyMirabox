#include <USBHost_t36.h>
#include "DeviceManager.h"

USBHost myusb;
DeviceManager dm(myusb);


const char *images[] = { "maarten.jpg", "maarten2.jpg", "maarten3.jpg", "maarten4.jpg", "maarten5.jpg", "maarten6.jpg", "maarten7.jpg"};
K1Pro *k1Pro = nullptr;
int brightness = 0.0;
int lighting_mode = 0;
int lighting_speed = 0;
bool color_select_mode = false;
int color[3] = {0,0,0};

void onAdded(StreamDock* device) {
  Serial.println("Device added");
  printf("ID: %s\n", device->id().c_str());
  printf("Serial: %s\n", device->get_serial_number().c_str());
  printf("Path: %s\n", device->getPath().c_str());
  
  printf("device->get_device_type() = %d\n", (int)device->get_device_type());
  if(device->get_device_type() == device_type::k1pro) {
    Serial.println("K1Pro setting to SDK Mode");
    k1Pro = (K1Pro*)(device);
    k1Pro->keyboard_mode(1);
  }

  device->clearAllIcon();
  const int image_count = (int)(sizeof(images) / sizeof(images[0]));
  for (int i = 0; i < device->image_keys(); i++) {
    device->set_key_image(i+1, images[i % image_count]);
  }
  device->set_key_callback(key_callback);
  device->refresh();
}

void onRemoved(StreamDock* d) {
  Serial.println("Device removed");
}

void key_callback(StreamDock *device, const InputEvent &event)
{
  if(!k1Pro) {
    Serial.println("Example requires a K1Pro");
    return;
  }
  switch(event.event_type) {
    case EventType::KNOB_ROTATE:
      if(!color_select_mode) {
        update_effects(event);
      } else {
        update_colors(event);
      }
      break;
    case EventType::KNOB_PRESS:
      color_select_mode = !color_select_mode;
      break;
    case EventType::BUTTON:
    case EventType::SWIPE:
    default:
      break;

  }
}

void update_effects(const InputEvent &event)
{
  int dir = event.direction == Direction::LEFT ? -1 : 1;
  switch(event.knob_id) {
    case KnobId::KNOB_1:
      brightness += dir;
      brightness = max(brightness, 0);
      brightness = min(brightness, 100);
      k1Pro->set_keyboard_backlight_brightness(brightness);
      break;
    case KnobId::KNOB_2:
      lighting_mode += dir;
      lighting_mode = max(lighting_mode, 0);
      lighting_mode = min(lighting_mode, 7);
      k1Pro->set_keyboard_lighting_effects(lighting_mode);
      break; 
    case KnobId::KNOB_3:
      lighting_speed += dir;
      lighting_speed = max(lighting_speed, 0);
      lighting_speed = min(lighting_speed, 9);
      k1Pro->set_keyboard_lighting_speed(lighting_speed);
      break;
    default:
      break;
  }
  printf("brightness = %i, lighting_mode = %i, lighting_speed = %i\n", brightness, lighting_mode, lighting_speed);
}

void update_colors(const InputEvent &event)
{
  if((int)event.knob_id < 0 || (int)event.knob_id >= 3) {
    return;
  }
  int dir = event.direction == Direction::LEFT ? -1 : 1;
  color[(int)event.knob_id] += 5 * dir;
  for(int i = 0; i < 3; i++) {
    color[i] = max(color[i], 0);
    color[i] = min(color[i], 255);
  }
  k1Pro->set_keyboard_rgb_backlight(color[0], color[1], color[2]);
  printf("Color = [%i, %i, %i]", color[0], color[1], color[2]);
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    // wait for the Serial Monitor so early prints are not lost
  }
  Serial.println("Mirabox K1Pro example starting");
  Serial.println("Connect a K1Pro, and use the knobs to set the brightness / mode / speed of the backlights.");
  Serial.println("Switch to color mode to set the RGB color via the knobs.");
  
  Serial.println("Press any know to toggle between settings mode and color mode.");
  dm.setDeviceChangeCallback(onAdded, onRemoved);
  dm.begin(true, true);

  MiraBoxHIDInput::show_raw_data = false;
  MiraBoxHIDInput::show_formated_data = false;
  myusb.begin();  // required
}
void loop() {
  dm.poll();      // calls host_.Task() internally

  static unsigned long last_status_ms = 0;
  if (millis() - last_status_ms >= 5000) {
    last_status_ms = millis();
    //Serial.printf("[status] alive, devices connected: %u\n", (unsigned)dm.deviceCount());
  }
}