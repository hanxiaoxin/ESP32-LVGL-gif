#include "settings.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ssid_manager.h>
#include <wifi_configuration_ap.h>
#include <wifi_station.h>

static const char *TAG = "WifiBoard";
static bool wifi_config_mode_ = false;

void wifiInit() {
  Settings settings("wifi", true);
  int force_dp = settings.GetInt("force_ap");
  // set force ap = 1
  if (force_dp) {
    ESP_LOGW(TAG, "force_ap is set to 1");
  } else {
    ESP_LOGW(TAG, "force_ap is set to 0");
  }
  wifi_config_mode_ = force_dp == 1;
  if (wifi_config_mode_) {
    ESP_LOGI(TAG, "force_ap is set to 1, reset to 0");
    settings.SetInt("force_ap", 0);
  }
}

void EnterWifiConfigMode() {
  auto &wifi_ap = WifiConfigurationAp::GetInstance();
  wifi_ap.SetSsidPrefix("Hanxiaoxin");
  wifi_ap.Start();

  // 显示 WiFi 配置 AP 的 SSID 和 Web 服务器 URL
  std::string hint = "手机连接热点";
  hint += wifi_ap.GetSsid();
  hint += "，浏览器访问 ";
  hint += wifi_ap.GetWebServerUrl();
  hint += "\n\n";

  // Wait forever until reset after configuration
  while (true) {
    int free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    int min_free_sram = heap_caps_get_minimum_free_size(MALLOC_CAP_INTERNAL);
    ESP_LOGI(TAG, "Free internal: %u minimal internal: %u", free_sram,
             min_free_sram);
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

void StartNetwork() {
  // User can press BOOT button while starting to enter WiFi configuration mode
  if (wifi_config_mode_) {
    EnterWifiConfigMode();
    return;
  }

  // If no WiFi SSID is configured, enter WiFi configuration mode
  auto &ssid_manager = SsidManager::GetInstance();
  auto ssid_list = ssid_manager.GetSsidList();
  if (ssid_list.empty()) {
    wifi_config_mode_ = true;
    EnterWifiConfigMode();
    return;
  }

  auto &wifi_station = WifiStation::GetInstance();
  wifi_station.Start();

  // Try to connect to WiFi, if failed, launch the WiFi configuration AP
  if (!wifi_station.WaitForConnected(60 * 1000)) {
    wifi_station.Stop();
    wifi_config_mode_ = true;
    EnterWifiConfigMode();
    return;
  }
}

void ResetWifiConfiguration() {
  Settings settings("wifi", true);
  settings.SetInt("force_ap", 1);
  vTaskDelay(pdMS_TO_TICKS(1000));
  // Reboot the device
  esp_restart();
}