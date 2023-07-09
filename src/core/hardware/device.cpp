#include "core/hardware/device.h"

#define TAG "device"

Device::Device()
    : display(Display(PCD8544_DC_PIN, PCD8544_CS_PIN, PCD8544_RST_PIN,
                      PCD8544_CONTRAST, PCD8544_BIAS)),
      led(Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800)),
      joystick(Joystick(JOYSTICK_VRX_PIN, JOYSTICK_VRY_PIN, JOYSTICK_SW_PIN)),
      radio(Radio(RADIO_CE_PIN, RADIO_CSN_PIN)),
      cc1101(CC1101(SPI_SCK_PIN, SPI_MOSI_PIN, CC1101_CSN_PIN, CC1101_GDO0_PIN,
                    CC1101_GDO1_PIN, CC1101_GDO2_PIN)) {}

void Device::initialize() {
  Serial.begin(115200);
  while (!Serial)
    ;

  ESP_LOGI(TAG, "Willy is starting...");
  printHardwareInfo();

  display.initialize();
  /*if (!radio.initialize()) {
    ESP_LOGE(TAG, "Radio initialization failed");
    return;
  }*/
  if (!cc1101.initialize()) {
    ESP_LOGE(TAG, "CC1101 initialization failed");
    return;
  }

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

void Device::boot() {
  Adafruit_PCD8544 *d = display.getDriver();
  d->fillScreen(1);
  d->display();

  led.setPixelColor(0, 255, 0, 0); // red pixel
  for (int i = 1; i <= LED_BRIGHTNESS; i++) {
    led.setBrightness(i);
    led.show();
    delay(100);
  }

  d->clearDisplay();
  d->display();
}

void Device::sync() { joystick.sync(); }

Display *Device::getDisplay() { return &display; }

Joystick *Device::getJoystick() { return &joystick; }

Radio *Device::getRadio() { return &radio; }

CC1101 *Device::getCC1101() { return &cc1101; }