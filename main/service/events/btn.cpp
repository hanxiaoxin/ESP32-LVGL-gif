#include "board/button.h"
#include "esp_log.h"
#include "wifi.h"

#define TAG "Event"

#define BOOT_BUTTON_GPIO GPIO_NUM_9
#define TOUCH_BUTTON_GPIO GPIO_NUM_10

Button boot_button_(BOOT_BUTTON_GPIO);
Button touch_button_(TOUCH_BUTTON_GPIO);

void initButtonEvents() {
  boot_button_.OnPressDown([]() {
    ESP_LOGI(TAG, "Boot button pressed");
    ResetWifiConfiguration();
  });

  touch_button_.OnPressDown([]() {
    ESP_LOGI(TAG, "Touch button pressed");
  });

  touch_button_.OnPressUp([]() {
    ESP_LOGI(TAG, "Touch button released");
  });
}