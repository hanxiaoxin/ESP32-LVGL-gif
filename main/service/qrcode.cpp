#include "esp_log.h"
#include "esp_netif.h"
#include <cJSON.h>
#include <esp_err.h>
#include <esp_http.h>
#include <esp_mac.h>
#include <iomanip>
#include <sstream>
#include <string>
#include "lib/ssd1306.h"

#define TAG "QRCODE"

std::string macToString(const uint8_t mac[6]) {
  std::ostringstream oss;
  for (int i = 0; i < 6; ++i) {
    if (i != 0)
      oss << ":";
    oss << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(mac[i]);
  }
  return oss.str();
}

DisplayData getQrcode() {
  DisplayData dd;

  // Get MAC and use it to generate a unique SSID
  uint8_t mac[6];
  ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_SOFTAP));
  std::string mac_str = macToString(mac);
  std::replace(mac_str.begin(), mac_str.end(), ':', '-');
  ESP_LOGW(TAG, "mac address: %s", mac_str.c_str());

  std::string url =
      std::format("http://static.hanxiaoxin.cn/qrcode-json/{}.json", mac_str.c_str());
  ESP_LOGW(TAG, "qrcode config url: %s", url.c_str());

  Http *http = new EspHttp();
  if (!http->Open("GET", url)) {
    ESP_LOGE(TAG, "Failed to open HTTP connection");
    delete http;
    return dd;
  }

  auto response = http->GetBody();
  http->Close();
  delete http;

  cJSON *root = cJSON_Parse(response.c_str());
  ESP_LOGI(TAG, "Response: %s", response.c_str());
  if (root == NULL) {
    ESP_LOGE(TAG, "Failed to parse JSON response");
    return dd;
  }

  cJSON *data = cJSON_GetObjectItem(root, "data");
  if (data == NULL) {
    ESP_LOGE(TAG, "Failed to parse JSON response: data");
    return dd;
  }

  if (cJSON_IsArray(data)) {
    int size = cJSON_GetArraySize(data);
    if (size > 512)
      size = 512; // 限制最大长度为 512

    for (int i = 0; i < size; ++i) {
      cJSON *item = cJSON_GetArrayItem(data, i);
      if (cJSON_IsNumber(item)) {
        dd.Data[i] = (uint8_t)(item->valueint);
      }
    }
  }

  cJSON *scl = cJSON_GetObjectItem(root, "scl");
  ESP_LOGI(TAG, "Response scl: %d", scl->valueint);
  dd.scl = (gpio_num_t)scl->valueint;

  cJSON *sda = cJSON_GetObjectItem(root, "sda");
  ESP_LOGI(TAG, "Response sda: %d", sda->valueint);
  dd.sda = (gpio_num_t)sda->valueint;

  cJSON *invert = cJSON_GetObjectItem(root, "invert");
  ESP_LOGI(TAG, "Response invert: %d", invert->valueint);
  dd.invert = invert->valueint;

  return dd;
}