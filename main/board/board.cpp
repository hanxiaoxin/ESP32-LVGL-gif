#include "board.h"
#include "assets/lang_config.h"
#include "config.h"
#include "display/display.h"
#include "display/lcd_display.h"
#include "display/oled_display.h"
#include "driver/i2c_master.h"
#include "esp_app_desc.h"
#include "font_awesome_symbols.h"
#include "settings.h"
#include "system_info.h"
#include <driver/spi_common.h>
#include <esp_chip_info.h>
#include <esp_log.h>
#include <esp_ota_ops.h>
#include <esp_random.h>
#include <wifi_station.h>

#define TAG "Board"
#define LCD_SPI_HOST SPI2_HOST

// LVGL
#ifdef OLED_DISPLAY
LV_FONT_DECLARE(font_puhui_14_1);
LV_FONT_DECLARE(font_awesome_14_1);
#endif

#ifdef LCD_DISPLAY
LV_FONT_DECLARE(font_puhui_16_4);
LV_FONT_DECLARE(font_awesome_16_4);
#endif

Board::Board()
    : boot_button_(BOOT_BUTTON_GPIO), touch_button_(TOUCH_BUTTON_GPIO) {
  Settings settings("board", true);
  uuid_ = settings.GetString("uuid");
  if (uuid_.empty()) {
    uuid_ = GenerateUuid();
    settings.SetString("uuid", uuid_);
  }
  ESP_LOGI(TAG, "UUID=%s SKU=%s", uuid_.c_str(), board_name.c_str());
  initButtonEvents();

  // ssd1306
  if (OLED_DISPLAY) {
    initOledDisplay();
  }

  if (LCD_DISPLAY) {
    initLcdDisplay();
  }
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

void Board::initOledDisplay() {
  i2c_master_bus_config_t i2c_mst_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = OLED_I2C_SDA,
      .scl_io_num = OLED_I2C_SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags = {.enable_internal_pullup = true, .allow_pd = false}};
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &display_i2c_bus_));

  esp_err_t err = probe_SSD1306();

  if (err != ESP_OK) {
    ESP_LOGE(TAG, "ssd1306 i2c not found");
    return;
  }

  ESP_LOGI(TAG, "OLED I2C initialized");

  // SSD1306 config
  esp_lcd_panel_io_i2c_config_t io_config = {
      .dev_addr = 0x3C,
      .on_color_trans_done = nullptr,
      .user_ctx = nullptr,
      .control_phase_bytes = 1,
      .dc_bit_offset = 6,
      .lcd_cmd_bits = 8,
      .lcd_param_bits = 8,
      .flags =
          {
              .dc_low_on_data = 0,
              .disable_control_phase = 0,
          },
      .scl_speed_hz = 400 * 1000,
  };

  ESP_ERROR_CHECK(
      esp_lcd_new_panel_io_i2c_v2(display_i2c_bus_, &io_config, &panel_io_));

  ESP_LOGI(TAG, "Install SSD1306 driver");
  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = -1;
  panel_config.bits_per_pixel = 1;

  esp_lcd_panel_ssd1306_config_t ssd1306_config = {
      .height = static_cast<uint8_t>(OLED_HEIGHT),
  };
  panel_config.vendor_config = &ssd1306_config;
  ESP_LOGI(TAG, "SSD1306 driver installed");
  ESP_ERROR_CHECK(esp_lcd_new_panel_ssd1306(panel_io_, &panel_config, &panel_));

  // Reset the display
  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_));
  if (esp_lcd_panel_init(panel_) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize display");
    return;
  }

  // Set the display to on
  ESP_LOGI(TAG, "Turning display on");
  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_, true));
  delete display_;
  display_ = new OledDisplay(panel_io_, panel_, OLED_WIDTH, OLED_HEIGHT,
                             DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y,
                             {&font_puhui_14_1, &font_awesome_14_1});
}

void Board::initLcdDisplay() {
  ESP_LOGW(TAG, "Free internal heap: %d\n",
           heap_caps_get_free_size(MALLOC_CAP_8BIT));

  ESP_LOGD(TAG, "Init SPI bus for LCD display");
  spi_bus_config_t buscfg = {};
  buscfg.mosi_io_num = LCD_SDA_PIN;
  buscfg.miso_io_num = GPIO_NUM_NC;
  buscfg.sclk_io_num = LCD_SCL_PIN;
  buscfg.quadwp_io_num = GPIO_NUM_NC;
  buscfg.quadhd_io_num = GPIO_NUM_NC;
  buscfg.max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * sizeof(uint16_t);
  esp_err_t ret = spi_bus_initialize(LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);

  if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "SPI bus init failed: %s", esp_err_to_name(ret));
    return;
  } else {
    ESP_LOGI(TAG, "SPI bus initialized successfully");
  }

  esp_lcd_panel_io_handle_t panel_io = nullptr;
  esp_lcd_panel_handle_t panel = nullptr;

  // 液晶屏控制IO初始化
  ESP_LOGI(TAG, "Install panel IO");
  esp_lcd_panel_io_spi_config_t io_config = {};
  io_config.cs_gpio_num = LCD_CS_PIN;
  io_config.dc_gpio_num = LCD_DC_PIN;
  io_config.spi_mode = LCD_SPI_MODE;
  io_config.pclk_hz = 40 * 1000 * 1000;
  io_config.trans_queue_depth = 10;
  io_config.lcd_cmd_bits = 8;
  io_config.lcd_param_bits = 8;
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_io_spi(LCD_SPI_HOST, &io_config, &panel_io));
  ESP_LOGI(TAG, "panel IO installed");

  // 初始化液晶屏驱动芯片
  ESP_LOGI(TAG, "Install LCD driver");
  esp_lcd_panel_dev_config_t panel_config = {};
  panel_config.reset_gpio_num = LCD_RST_PIN;
  panel_config.rgb_ele_order = LCD_RGB_ORDER;
  panel_config.bits_per_pixel = 16;

  // LCD 驱动芯片配置
  ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(panel_io, &panel_config, &panel));
  ESP_LOGI(TAG, "LCD driver installed");

  esp_lcd_panel_reset(panel);

  esp_lcd_panel_init(panel);
  esp_lcd_panel_invert_color(panel, DISPLAY_INVERT_COLOR);
  esp_lcd_panel_swap_xy(panel, DISPLAY_SWAP_XY);
  esp_lcd_panel_mirror(panel, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y);

  const lv_font_t *emoji_font =
      LCD_HEIGHT >= 240 ? font_emoji_64_init() : font_emoji_32_init();

  display_ = new SpiLcdDisplay(
      panel_io, panel, LCD_WIDTH, LCD_HEIGHT, DISPLAY_OFFSET_X,
      DISPLAY_OFFSET_Y, DISPLAY_MIRROR_X, DISPLAY_MIRROR_Y, DISPLAY_SWAP_XY,
      {
          .text_font = &font_puhui_16_4,
          .icon_font = &font_awesome_16_4,
          .emoji_font = emoji_font,
      });
  ESP_LOGI(TAG, "LCD display initialized");
  ESP_LOGW(TAG, "Free internal heap: %d\n",
           heap_caps_get_free_size(MALLOC_CAP_8BIT));
}

esp_err_t Board::probe_SSD1306() {
  int count = 10;
  esp_err_t err;
  while (count) {
    err = i2c_master_probe(display_i2c_bus_, OLED_I2C_ADDRESS, 50);

    if (err == ESP_OK) {
      return err;
    }

    count--;
  }

  return err;
}

bool Board::GetBatteryLevel(int &level, bool &charging, bool &discharging) {
  level = 100;
  charging = false;
  discharging = false;
  return true;
}