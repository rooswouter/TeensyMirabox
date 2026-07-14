/*
  Example usage for a Mirabox device as a joystick 
  Mirabox Joystick example starting
  Connect a Mirabox device, which can be used as a joystick.
  The knobs control the axis, press knob to zero axis.
  Swipe direction sets the hat direction
*/

#include <USBHost_t36.h>
#include "DeviceManager.h"

USBHost myusb;
DeviceManager dm(myusb);

float axis[4] = {0.0, 0.0, 0.0, 0.0};

void onAdded(StreamDock* device) {
  Serial.println("Device added");
  printf("ID: %s\n", device->id().c_str());
  printf("Serial: %s\n", device->get_serial_number().c_str());
  printf("Path: %s\n", device->getPath().c_str());
  printf("device->get_device_type() = %d\n", (int)device->get_device_type());
  device->clearAllIcon();
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
      Joystick.button((int)event.key, event.state); 
      break;
    case EventType::KNOB_ROTATE:
      if((int)event.knob_id >= 0 && (int)event.knob_id <= 3) {
        axis[(int)event.knob_id] =  axis[(int)event.knob_id] + ((int)event.direction == 0 ? - 0.05f : + 0.05f);
      }
      break;
    case EventType::KNOB_PRESS:
      if((int)event.knob_id >= 0 && (int)event.knob_id <= 3) {
        axis[(int)event.knob_id] = 0.0f;
      }
      break;
    case EventType::SWIPE:
      {
        int angle = 0;
        switch(event.direction)
        {
          case Direction::LEFT:
            angle = 270;
            break;
          case Direction::RIGHT:
            angle = 90;
            break;
          case Direction::UP:
            angle = 0;
            break;
          case Direction::DOWN:
            angle = 180;
            break;
        }  
        Joystick.hat(angle);          // "angle" is 0,45,90,135,180,225,270,315,-1
      }
      break;
    default:
      break;

  }

}

void send_axis()
{
  axis[0] = std::clamp(axis[0], -1.0f, 1.0f);
  axis[1] = std::clamp(axis[1], -1.0f, 1.0f);
  axis[2] = std::clamp(axis[2], 0.0f, 1.0f);
  axis[3] = std::clamp(axis[3], 0.0f, 1.0f);
  
  
  Joystick.X(512 * axis[0] + 512);
  Joystick.Y(512 * axis[1] + 512);
  Joystick.sliderLeft(1024 * axis[2]);
  Joystick.sliderRight(1024 * axis[3]);
  
}
void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    // wait for the Serial Monitor so early prints are not lost
  }
  Serial.println("Mirabox Joystick example starting");
  Serial.println("Connect a Mirabox device, which can be used as a joystick.");
  Serial.println("The knobs control the axis, press knob to zero axis.");
  
  dm.setDeviceChangeCallback(onAdded, onRemoved);
  dm.begin(true, true);

  MiraBoxHIDInput::show_raw_data = false;
  MiraBoxHIDInput::show_formated_data = false;
  myusb.begin();  // required
  Joystick.useManualSend(true);
}

void loop() {
  dm.poll();      // calls host_.Task() internally
  delay(10);
  send_axis();
  Joystick.send_now();
}
