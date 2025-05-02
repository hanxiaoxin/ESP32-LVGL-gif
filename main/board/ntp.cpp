#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "application.h"
#include "board/board.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "lang_config.h"
#include "nvs_flash.h"

static const char *TAG = "NTP";

struct tm timeinfo;

void time_sync_notification_cb(struct timeval *tv) {
  ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void obtain_time(void) {
  ESP_LOGI(TAG, "Initializing SNTP");

  esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
  esp_sntp_setservername(0, "pool.ntp.org"); // 使用全局NTP服务器
  sntp_set_time_sync_notification_cb(time_sync_notification_cb);
  esp_sntp_init();

  // 等待时间同步完成
  time_t now = 0;
  struct tm timeinfo = {};
  int retry = 0;
  const int retry_count = 10;

  while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count) {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry,
             retry_count);
    vTaskDelay(pdMS_TO_TICKS(2000));
    time(&now);
    localtime_r(&now, &timeinfo);
    setenv("TZ", "CST-8", 1);
    tzset();
  }
}

void init_ntp(){
  ESP_LOGI(TAG, "Startup..");
  // 同步时间
  obtain_time();

  // 获取并打印当前时间
  time_t now;
  time(&now);
  localtime_r(&now, &timeinfo);
  char strftime_buf[64];
  strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);
  ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);

  auto display_ = Board::GetInstance().GetDisplay();

  // Success
  display_->SetStatus(Lang::Strings::CONNECTED_TO);
  Application::GetInstance().SetDeviceState(DeviceState::kDeviceStateReady);
}