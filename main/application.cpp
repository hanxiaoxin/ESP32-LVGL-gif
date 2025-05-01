#include "application.h"
#include "background_task.h"
#include "board/board.h"
#include "board/ntp.h"
#include "board/serial.h"
#include "lang_config.h"
#include "led/led.h"
#include "service.h"
#include "wifi.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define TAG "Application"

Application::Application() {}

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

  Board &board = Board::GetInstance();
  // StartNetwork();
  // init_ntp();
  init_uart();
  xTaskCreatePinnedToCore(led_blink, "led_blink", 4096, NULL, 5, NULL, 0);
  runServices();
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

void Application::SetDeviceState(DeviceState state) {
  if (device_state_ == state) {
    return;
  }

  device_state_ = state;
  // The state is changed, wait for all background tasks to finish
  background_task_->WaitForCompletion();

  auto &board = Board::GetInstance();
  auto display = board.GetDisplay();

  switch (state) {
    case kDeviceStateUnknown:
    case kDeviceStateIdle:
      display->SetStatus(Lang::Strings::STANDBY);
      break;
    case kDeviceStateConnecting:
      display->SetStatus(Lang::Strings::CONNECTING);
      break;
    case kDeviceStateListening:
      display->SetStatus(Lang::Strings::LISTENING);
      break;
    case kDeviceStateSpeaking:
      display->SetStatus(Lang::Strings::SPEAKING);
      break;
    default:
      // Do nothing
      break;
  }
}