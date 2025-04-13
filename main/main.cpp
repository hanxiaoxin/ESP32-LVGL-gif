#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ssd1306.h"

#define TAG "Application"

void initSSD1306(void *params) {
  SSD1306 &display = SSD1306::GetInstance();

  esp_err_t err = display.probeSSD1306();

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ssd1306 i2c not found");
    vTaskDelete(NULL);
    return;
  }

  display.init();
  display.clear();
  uint8_t myData[5] = {0xFF, 0x81, 0xBD, 0x81, 0xFF}; // 字母 A 形状
  display.display_image(2, 10, myData, 5);
  vTaskDelete(NULL);
}

extern "C" void app_main(void) {

  xTaskCreatePinnedToCore(initSSD1306, "init ssd1306", 4096 * 2, NULL, 1, NULL,
                          0);
}