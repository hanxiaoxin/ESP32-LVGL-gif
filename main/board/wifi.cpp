#include "esp_wifi.h"
#include "settings.h"
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ssid_manager.h>
#include <wifi_configuration_ap.h>
#include <wifi_station.h>
#include "font_awesome_symbols.h"
#include "board/board.h"
#include "lang_config.h"
#include "application.h"

static const char *TAG = "WifiBoard";
static bool wifi_config_mode_ = false;

#define DEFAULT_WIFI 0
#define LOW_WIFI_POWER 0

void set_wifi_power() {
  esp_err_t err = esp_wifi_set_max_tx_power(34);
  if (err != ESP_OK) {
    ESP_LOGE("WIFI", "Failed to set TX power: %s", esp_err_to_name(err));
  } else {
    ESP_LOGI("WIFI", "WiFi TX power set to 8.5dBm");
  }
}

void EnterWifiConfigMode() {
  Application::GetInstance().SetDeviceState(DeviceState::kDeviceStateWifiConfiguring);
  auto display_ = Board::GetInstance().GetDisplay();
  auto &wifi_ap = WifiConfigurationAp::GetInstance();
  wifi_ap.SetSsidPrefix("Hanxiaoxin");
  wifi_ap.Start();

  // 显示 WiFi 配置 AP 的 SSID 和 Web 服务器 URL
  std::string hint = "手机连接热点";
  hint += wifi_ap.GetSsid();
  hint += "，浏览器访问 ";
  hint += wifi_ap.GetWebServerUrl();
  hint += "\n\n";

  display_->SetStatus(Lang::Strings::CONNECT_TO_HOTSPOT);
  display_->SetIcon(FONT_AWESOME_EMOJI_THINKING);
  display_->SetChatMessage("",hint.c_str());

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
  Settings settings("wifi", true);
  wifi_config_mode_ = settings.GetInt("force_ap") == 1;
  if (wifi_config_mode_) {
    ESP_LOGI(TAG, "force_ap is set to 1, reset to 0");
    settings.SetInt("force_ap", 0);
    wifi_config_mode_ = true;
  }

  Application::GetInstance().SetDeviceState(
      DeviceState::kDeviceStateConnecting);
  if (DEFAULT_WIFI) {
    settings.SetString("ssid", "iKuai2G-cc7b");
    settings.SetString("password", "314314314");
    ESP_LOGW(TAG, "Default WiFi SSID: %s", settings.GetString("ssid").c_str());
  }

  // User can press BOOT button while starting to enter WiFi configuration
  // mode
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

  if (LOW_WIFI_POWER) {
    set_wifi_power();
  }

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