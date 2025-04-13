#include "esp_err.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lib/ssd1306.h"
#include "nvs_flash.h"
#include "wifi.h"
#include "service/qrcode.h"

#define MAX_HTTP_RECV_BUFFER 2048

#define TAG "Application"


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

extern "C" void app_main(void) {
  // Initialize NVS flash for WiFi configuration
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
      ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "Erasing NVS flash to fix corruption");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_ERROR_CHECK(esp_event_loop_create_default());

  StartNetwork();

  initSSD1306();
}