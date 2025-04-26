#include "bangocat/cat.h"
#include "display/ssd1306.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "qrcode/qrcode.h"
#include "ws2812/ws2812.h"
#include "clock/clock.h"

#define TAG "SERVICE"

void qrcode_ssd1306() {
  DisplayData dd = getQrcode();

  SSD1306 &display = SSD1306::GetInstance(dd.scl, dd.sda, dd.invert);

  esp_err_t err = display.probe_SSD1306();

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ssd1306 i2c not found");
    vTaskDelete(NULL);
    return;
  }

  display.invert(1);
  display.init();
  display.clear();

  ESP_LOGI(TAG, "invert: %d", display._invert);
  display.invert(display._invert);

  for (int page = 0; page < 8; page++) {
    display.display_image(page, 32, &dd.Data[page * 64], 64);
  }

  vTaskDelete(NULL);
}

void gif_ssd1306() {
  SSD1306 &display = SSD1306::GetInstance();

  esp_err_t err = display.probe_SSD1306();

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ssd1306 i2c not found");
    vTaskDelete(NULL);
    return;
  }

  showBangoCat(display);
}

void clock_ssd1306(){
  SSD1306 &display = SSD1306::GetInstance();

  esp_err_t err = display.probe_SSD1306();

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ssd1306 i2c not found");
    vTaskDelete(NULL);
    return;
  }

  showClock(display);
}

void runServices() {
  ESP_LOGI(TAG, "Starting services");
  // gif_ssd1306();
  // ws2812_init();
  clock_ssd1306();
}