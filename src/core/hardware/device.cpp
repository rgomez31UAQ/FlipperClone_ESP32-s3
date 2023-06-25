#include "core/hardware/device.h"

#define TAG "device"

Device::Device()
    : display(
          Adafruit_PCD8544(PCD8544_DC_PIN, PCD8544_CS_PIN, PCD8544_RST_PIN)),
      led(Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800)),
      joystick(Joystick(JOYSTICK_VRX_PIN, JOYSTICK_VRY_PIN, JOYSTICK_SW_PIN)),
      radio(RF24(RADIO_CE_PIN, RADIO_CSN_PIN)) {}

void Device::initialize() {
  Serial.begin(115200);
  while (!Serial)
    ;

  ESP_LOGI(TAG, "Willy is starting...");
  printHardwareInfo();

  initializeDisplay();
  initializeRadio();
  boot();
}

void Device::printHardwareInfo() {
  ESP_LOGI(TAG, "Chip model: %s rev. %d", ESP.getChipModel(),
           ESP.getChipRevision());
  ESP_LOGI(TAG, "CPU cores: %d", ESP.getChipCores());

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  ESP_LOGI(TAG, "Flash: %dMB %s", spi_flash_get_chip_size() / (1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded"
                                                         : "external");
  ESP_LOGI(TAG, "Flash memory embedded: %s",
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "yes" : "no");
  ESP_LOGI(TAG, "2.4GHz WiFi: %s",
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "yes" : "no");
  ESP_LOGI(TAG, "Bluetooth LE: %s",
           (chip_info.features & CHIP_FEATURE_BLE) ? "yes" : "no");
  ESP_LOGI(TAG, "Bluetooth Classic: %s",
           (chip_info.features & CHIP_FEATURE_BT) ? "yes" : "no");
  ESP_LOGI(TAG, "IEEE 802.15.4: %s",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? "yes" : "no");
  ESP_LOGI(TAG, "PSRAM embedded: %s",
           (chip_info.features & CHIP_FEATURE_EMB_PSRAM) ? "yes" : "no");
  ESP_LOGI(TAG, "Silicon revision: %d", chip_info.revision);

  uint64_t chipID = ESP.getEfuseMac();
  ESP_LOGI(TAG, "Chip ID: %04X", (uint16_t)(chipID >> 32));
  ESP_LOGI(TAG, "MAC address: %08X", (uint32_t)chipID);

  ESP_LOGI(TAG, "CPU temperature: %.2f C", temperatureRead());
}

void Device::initializeDisplay() {
  display.begin();
  display.setRotation(2);
  display.setContrast(PCD8544_CONTRAST);
  display.setBias(PCD8544_BIAS);
  display.clearDisplay();
  display.display();
}

void Device::initializeRadio() {
  if (!radio.begin()) {
    ESP_LOGE(TAG, "Radio hardware is not responding.");
    return;
  }

  uint8_t rxAddr[5] = "ardu";

  radio.setPALevel(RF24_PA_HIGH);
  radio.openWritingPipe(rxAddr);
  radio.setChannel(5);
  radio.setAutoAck(true);
  radio.enableAckPayload();
  radio.enableDynamicPayloads();
  radio.setRetries(15, 15);
  radio.printDetails();

  const byte buffer[] = "bazant";
  bool ok = radio.write(buffer, 6);
  if (ok) {
    ESP_LOGI(TAG, "Message sent OK.");
  } else if (radio.failureDetected) {
    ESP_LOGE(TAG, "Radio: failure detected");
  } else {
    ESP_LOGE(TAG, "Message not sent, failed.");
  }

  int rec[1] = {5};
  if (radio.isAckPayloadAvailable()) {
    radio.read(rec, sizeof(rec));
    ESP_LOGI(TAG, "received ack payload is: %d", rec[0]);
  }
}

void Device::boot() {
  display.fillScreen(1);
  display.display();

  led.setPixelColor(0, 255, 0, 0); // red pixel
  for (int i = 1; i <= LED_BRIGHTNESS; i++) {
    led.setBrightness(i);
    led.show();
    delay(100);
  }

  display.clearDisplay();
  display.display();
}

void Device::sync() { joystick.sync(); }

Adafruit_PCD8544 *Device::getDisplay() { return &display; }

Joystick *Device::getJoystick() { return &joystick; }