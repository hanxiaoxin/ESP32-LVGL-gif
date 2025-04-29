#include "board.h"
#include "assets/lang_config.h"
#include "display/display.h"
#include "font_awesome_symbols.h"
#include "settings.h"
#include "system_info.h"
#include <esp_chip_info.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <wifi_station.h>
#include "config.h"

#define TAG "Board"

Board::Board():boot_button_(BOOT_BUTTON_GPIO),
    touch_button_(TOUCH_BUTTON_GPIO) {
  Settings settings("board", true);
  uuid_ = settings.GetString("uuid");
  if (uuid_.empty()) {
    uuid_ = GenerateUuid();
    settings.SetString("uuid", uuid_);
  }
  ESP_LOGI(TAG, "UUID=%s SKU=%s", uuid_.c_str(), board_name.c_str());
  initButtonEvents();
}

std::string Board::GetBoardType() { return board_type; }

std::string Board::GenerateUuid() {
  // UUID v4 需要 16 字节的随机数据
  uint8_t uuid[16];

  // 使用 ESP32 的硬件随机数生成器
  esp_fill_random(uuid, sizeof(uuid));

  // 设置版本 (版本 4) 和变体位
  uuid[6] = (uuid[6] & 0x0F) | 0x40; // 版本 4
  uuid[8] = (uuid[8] & 0x3F) | 0x80; // 变体 1

  // 将字节转换为标准的 UUID 字符串格式
  char uuid_str[37];
  snprintf(
      uuid_str, sizeof(uuid_str),
      "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
      uuid[0], uuid[1], uuid[2], uuid[3], uuid[4], uuid[5], uuid[6], uuid[7],
      uuid[8], uuid[9], uuid[10], uuid[11], uuid[12], uuid[13], uuid[14],
      uuid[15]);

  return std::string(uuid_str);
}

Display *Board::GetDisplay() { return display_; }

std::string Board::GetJSON() {
  /*
      {
          "version": 2,
          "flash_size": 4194304,
          "psram_size": 0,
          "minimum_free_heap_size": 123456,
          "mac_address": "00:00:00:00:00:00",
          "uuid": "00000000-0000-0000-0000-000000000000",
          "chip_model_name": "esp32s3",
          "chip_info": {
              "model": 1,
              "cores": 2,
              "revision": 0,
              "features": 0
          },
          "application": {
              "name": "my-app",
              "version": "1.0.0",
              "compile_time": "2021-01-01T00:00:00Z"
              "idf_version": "4.2-dev"
              "elf_sha256": ""
          },
          "partition_table": [
              "app": {
                  "label": "app",
                  "type": 1,
                  "subtype": 2,
                  "address": 0x10000,
                  "size": 0x100000
              }
          ],
          "ota": {
              "label": "ota_0"
          },
          "board": {
              ...
          }
      }
  */
  std::string json = "{";
  json += "\"version\":2,";
  json += "\"language\":\"" + std::string(Lang::CODE) + "\",";
  json += "\"flash_size\":" + std::to_string(SystemInfo::GetFlashSize()) + ",";
  json += "\"minimum_free_heap_size\":" +
          std::to_string(SystemInfo::GetMinimumFreeHeapSize()) + ",";
  json += "\"mac_address\":\"" + SystemInfo::GetMacAddress() + "\",";
  json += "\"uuid\":\"" + uuid_ + "\",";
  json += "\"chip_model_name\":\"" + SystemInfo::GetChipModelName() + "\",";
  json += "\"chip_info\":{";

  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  json += "\"model\":" + std::to_string(chip_info.model) + ",";
  json += "\"cores\":" + std::to_string(chip_info.cores) + ",";
  json += "\"revision\":" + std::to_string(chip_info.revision) + ",";
  json += "\"features\":" + std::to_string(chip_info.features);
  json += "},";

  json += "\"application\":{";
  auto app_desc = esp_app_get_description();
  json += "\"name\":\"" + std::string(app_desc->project_name) + "\",";
  json += "\"version\":\"" + std::string(app_desc->version) + "\",";
  json += "\"compile_time\":\"" + std::string(app_desc->date) + "T" +
          std::string(app_desc->time) + "Z\",";
  json += "\"idf_version\":\"" + std::string(app_desc->idf_ver) + "\",";

  char sha256_str[65];
  for (int i = 0; i < 32; i++) {
    snprintf(sha256_str + i * 2, sizeof(sha256_str) - i * 2, "%02x",
             app_desc->app_elf_sha256[i]);
  }
  json += "\"elf_sha256\":\"" + std::string(sha256_str) + "\"";
  json += "},";

  json += "\"partition_table\": [";
  esp_partition_iterator_t it = esp_partition_find(
      ESP_PARTITION_TYPE_ANY, ESP_PARTITION_SUBTYPE_ANY, NULL);
  while (it) {
    const esp_partition_t *partition = esp_partition_get(it);
    json += "{";
    json += "\"label\":\"" + std::string(partition->label) + "\",";
    json += "\"type\":" + std::to_string(partition->type) + ",";
    json += "\"subtype\":" + std::to_string(partition->subtype) + ",";
    json += "\"address\":" + std::to_string(partition->address) + ",";
    json += "\"size\":" + std::to_string(partition->size);
    json += "},";
    it = esp_partition_next(it);
  }
  json.pop_back(); // Remove the last comma
  json += "],";

  json += "\"ota\":{";
  auto ota_partition = esp_ota_get_running_partition();
  json += "\"label\":\"" + std::string(ota_partition->label) + "\"";
  json += "},";

  json += "\"board\":" + GetBoardJson();

  // Close the JSON object
  json += "}";
  return json;
}

std::string Board::GetBoardJson() {
  // Set the board type for OTA
  auto &wifi_station = WifiStation::GetInstance();
  std::string board_json = std::string("{\"type\":\"" + board_type + "\",");
  board_json += "\"name\":\"" + board_name + "\",";
  board_json += "\"ssid\":\"" + wifi_station.GetSsid() + "\",";
  board_json += "\"rssi\":" + std::to_string(wifi_station.GetRssi()) + ",";
  board_json +=
      "\"channel\":" + std::to_string(wifi_station.GetChannel()) + ",";
  board_json += "\"ip\":\"" + wifi_station.GetIpAddress() + "\",";
  board_json += "\"mac\":\"" + SystemInfo::GetMacAddress() + "\"}";
  return board_json;
}

const char *Board::GetNetworkStateIcon() {
  auto &wifi_station = WifiStation::GetInstance();
  if (!wifi_station.IsConnected()) {
    return FONT_AWESOME_WIFI_OFF;
  }
  int8_t rssi = wifi_station.GetRssi();
  if (rssi >= -60) {
    return FONT_AWESOME_WIFI;
  } else if (rssi >= -70) {
    return FONT_AWESOME_WIFI_FAIR;
  } else {
    return FONT_AWESOME_WIFI_WEAK;
  }
}

void Board::initButtonEvents() {
  ESP_LOGI(TAG, "Initializing button events");
  boot_button_.OnPressDown([]() { ESP_LOGI(TAG, "PRESS_DOWN triggered"); });
  boot_button_.OnPressUp([]() { ESP_LOGI(TAG, "PRESS_UP triggered"); });
  boot_button_.OnLongPress([]() { ESP_LOGI(TAG, "LONG_PRESS triggered"); });
  boot_button_.OnClick([]() { ESP_LOGI(TAG, "CLICK triggered"); });

  touch_button_.OnPressDown([]() { ESP_LOGI(TAG, "Touch button pressed"); });
  touch_button_.OnClick([]() { ESP_LOGI(TAG, "Touch button click"); });
  touch_button_.OnPressUp([]() { ESP_LOGI(TAG, "Touch button released"); });
}