#include "application.h"
#include "board/button.h"
#include "led/led_pwm.h"
#include "wifi.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "Application"

Application::Application()
    : boot_button_(BOOT_BUTTON_GPIO), touch_button_(TOUCH_BUTTON_GPIO) {}

Application::~Application() {}

void Application::Start() {
  initButtonEvents();
  StartNetwork();
  
  pwm_led_init();
  xTaskCreatePinnedToCore(pwm_blink, "ledInit", 4096, NULL, 5, NULL, 0);
}

void Application::initButtonEvents() {
  ESP_LOGI(TAG, "Initializing button events");
  boot_button_.OnPressDown([]() { ESP_LOGI(TAG, "PRESS_DOWN triggered"); });
  boot_button_.OnPressUp([]() { ESP_LOGI(TAG, "PRESS_UP triggered"); });
  boot_button_.OnLongPress([]() { ESP_LOGI(TAG, "LONG_PRESS triggered"); });
  boot_button_.OnClick([]() { ESP_LOGI(TAG, "CLICK triggered"); });

  touch_button_.OnPressDown([]() { ESP_LOGI(TAG, "Touch button pressed"); });
  touch_button_.OnClick([]() { ESP_LOGI(TAG, "Touch button click"); });
  touch_button_.OnPressUp([]() { ESP_LOGI(TAG, "Touch button released"); });
}