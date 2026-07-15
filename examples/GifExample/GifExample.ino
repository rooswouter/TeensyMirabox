#include <USBHost_t36.h>
#include <MiraBox.h>


USBHost myusb;
DeviceManager dm(myusb);

void onAdded(StreamDock *device) {
  Serial.println("Device added");
  device->init();

  if (device->set_key_gif(1, "anim.gif") == 0) {
      Serial.println("Key GIF loaded");
      GifSharedStream shared = device->export_key_gif_stream(1);
      for(int i = 2; i <= device->image_keys(); i++) {
        device->set_key_gif_shared(i, shared);
      }
  } else {
      Serial.println("set_key_gif failed (WITH_ANIMATEDGIF defined? GIF on SD?)");
  }
  device->set_key_callback(key_callback);
}

void key_callback(StreamDock *device, const InputEvent &event)
{
  switch(event.event_type) {
    case EventType::BUTTON:
      if(event.state == 1) {
        if(device->gif_loop_status()) {
          device->stop_gif_loop();
        } else {
          device->start_gif_loop();
        }
      }
      break;
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {}
  Serial.println("Mirabox GifExample Started, waiting for device...");
  dm.setDeviceChangeCallback(onAdded, nullptr);
  dm.begin(true, true);
  myusb.begin();
}

void loop() {
  dm.poll();
}
