#include "display/ssd1306.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "qrcode/qrcode.h"

#define TAG "SERVICE"

void initSSD1306() {
  DisplayData dd = getQrcode();

  SSD1306 &display = SSD1306::GetInstance(dd.scl, dd.sda, dd.invert);

  esp_err_t err = display.probe_SSD1306();

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ssd1306 i2c not found");
    vTaskDelete(NULL);
    return;
  }

  display.init();
  display.clear();

  ESP_LOGI(TAG, "invert: %d", display._invert);
  display.invert(display._invert);

  for (int page = 0; page < 8; page++) {
    display.display_image(page, 32, &dd.Data[page * 64], 64);
  }

  vTaskDelete(NULL);
}