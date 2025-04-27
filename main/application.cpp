#include "application.h"
#include "board/button.h"
#include "board/ntp.h"
#include "board/serial.h"
#include "led/led.h"
#include "service.h"
#include "wifi.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "background_task.h"

#define TAG "Application"

Application::Application()
    : boot_button_(BOOT_BUTTON_GPIO), touch_button_(TOUCH_BUTTON_GPIO) {}

Application::~Application() {}

void Application::Start() {
  event_group_ = xEventGroupCreate();
  background_task_ = new BackgroundTask(4096 * 8);

  esp_timer_create_args_t clock_timer_args = {.callback =
                                                  [](void *arg) {
                                                    Application *app =
                                                        (Application *)arg;
                                                    app->OnClockTimer();
                                                  },
                                              .arg = this,
                                              .dispatch_method = ESP_TIMER_TASK,
                                              .name = "clock_timer",
                                              .skip_unhandled_events = true};
  esp_timer_create(&clock_timer_args, &clock_timer_handle_);
  esp_timer_start_periodic(clock_timer_handle_, 1000000);

  initButtonEvents();
  StartNetwork();
  init_ntp();
  init_uart();
  xTaskCreatePinnedToCore(led_blink, "led_blink", 4096, NULL, 5, NULL, 0);
  runServices();
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

void Application::OnClockTimer() {
  clock_ticks_++;

  // Print the debug info every 10 seconds
  if (clock_ticks_ % 10 == 0) {
    // SystemInfo::PrintRealTimeStats(pdMS_TO_TICKS(1000));

    int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    int min_free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "Free internal: %u minimal internal: %u", free_sram,
             min_free_sram);
  }
}