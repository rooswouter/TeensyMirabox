#include <USBHost_t36.h>
#include <MiraBox.h>

USBHost myusb;
DeviceManager dm(myusb);

void onAdded(StreamDock *device) {
  Serial.println("Device added");
  device->init();

  for(int i = 2; i <= 2; i++) {
    // Place a device-sized GIF on the SD card, e.g. StreamDockN3/key/anim.gif (64x64)
    if (device->set_key_gif(i, "anim.gif") == 0) {
      Serial.println("Key GIF loaded");
    } else {
      Serial.println("set_key_gif failed (ENABLE_ANIMATEDGIF defined? GIF on SD?)");
    }
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
