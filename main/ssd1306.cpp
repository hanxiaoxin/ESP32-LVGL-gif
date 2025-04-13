#include "ssd1306.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <string.h>

#define I2C_SCL_NUM GPIO_NUM_4
#define I2C_SDA_NUM GPIO_NUM_3
#define IC2_MASTER_FREQ_HZ 400000
#define I2C_TICKS_TO_WAIT 100
#define I2C_NUM I2C_NUM_0

#define TAG "SSD1306"
#include "driver/i2c_master.h"
#include "esp_log.h"

SSD1306::SSD1306() {
  ESP_LOGI(TAG, "New i2c bus is creating");
  i2c_master_bus_config_t i2c_mst_config = {
      .i2c_port = I2C_NUM,
      .sda_io_num = I2C_SDA_NUM,
      .scl_io_num = I2C_SCL_NUM,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags = {.enable_internal_pullup = true, .allow_pd = false}};
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

  i2c_device_config_t dev_cfg = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                 .device_address = I2C_ADDRESS,
                                 .scl_speed_hz = IC2_MASTER_FREQ_HZ,
                                 .scl_wait_us = 0,
                                 .flags = {.disable_ack_check = false}};

  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
}

SSD1306::~SSD1306() {}

void SSD1306::writeCommand(uint8_t command) {
  uint8_t cmd[2] = {0x00, command}; // 前面加上 0x00 表示命令模式
  // ESP_LOGW(TAG, "write command: %d", command);
  i2c_master_transmit(dev_handle, cmd, 2, 20 / portTICK_PERIOD_MS);
}

void SSD1306::init() {
  uint8_t out_buf[27];
  int out_index = 0;
  out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
  out_buf[out_index++] = OLED_CMD_DISPLAY_OFF;   // AE
  out_buf[out_index++] = OLED_CMD_SET_MUX_RATIO; // A8
  if (_height == 64)
    out_buf[out_index++] = 0x3F;
  if (_height == 32)
    out_buf[out_index++] = 0x1F;
  out_buf[out_index++] = OLED_CMD_SET_DISPLAY_OFFSET; // D3
  out_buf[out_index++] = 0x00;
  // out_buf[out_index++] = OLED_CONTROL_BYTE_DATA_STREAM;	// 40
  out_buf[out_index++] = OLED_CMD_SET_DISPLAY_START_LINE; // 40
  // out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP;		// A1
  if (_flip) {
    out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_0; // A0
  } else {
    out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_1; // A1
  }
  out_buf[out_index++] = OLED_CMD_SET_COM_SCAN_MODE;   // C8
  out_buf[out_index++] = OLED_CMD_SET_DISPLAY_CLK_DIV; // D5
  out_buf[out_index++] = 0x80;
  out_buf[out_index++] = OLED_CMD_SET_COM_PIN_MAP; // DA
  if (_height == 64)
    out_buf[out_index++] = 0x12;
  if (_height == 32)
    out_buf[out_index++] = 0x02;
  out_buf[out_index++] = OLED_CMD_SET_CONTRAST; // 81
  out_buf[out_index++] = 0xFF;
  out_buf[out_index++] = OLED_CMD_DISPLAY_RAM;       // A4
  out_buf[out_index++] = OLED_CMD_SET_VCOMH_DESELCT; // DB
  out_buf[out_index++] = 0x40;
  out_buf[out_index++] = OLED_CMD_SET_MEMORY_ADDR_MODE; // 20
  // out_buf[out_index++] = OLED_CMD_SET_HORI_ADDR_MODE;	// 00
  out_buf[out_index++] = OLED_CMD_SET_PAGE_ADDR_MODE; // 02
  // Set Lower Column Start Address for Page Addressing Mode
  out_buf[out_index++] = 0x00;
  // Set Higher Column Start Address for Page Addressing Mode
  out_buf[out_index++] = 0x10;
  out_buf[out_index++] = OLED_CMD_SET_CHARGE_PUMP; // 8D
  out_buf[out_index++] = 0x14;
  out_buf[out_index++] = OLED_CMD_DEACTIVE_SCROLL; // 2E
  out_buf[out_index++] = OLED_CMD_DISPLAY_NORMAL;  // A6
  out_buf[out_index++] = OLED_CMD_DISPLAY_ON;      // AF

  esp_err_t res;
  res = i2c_master_transmit(dev_handle, out_buf, out_index, I2C_TICKS_TO_WAIT);
  if (res == ESP_OK) {
    ESP_LOGI(TAG, "OLED configured successfully");
  } else {
    ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)",
             I2C_ADDRESS, I2C_NUM, res, esp_err_to_name(res));
  }
}

void SSD1306::full() {
  memset(_buffer + 1, 0XFF, _bufferSize - 1); // 全白
  flush();
}

void SSD1306::clear() {
  for (int page = 0; page < _pages; page++) {
    uint8_t blank[_width];
    memset(blank, 0x00, _width); // 全部清空

    display_image(page, 0, blank, _width);
  }
}

void SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || x >= _width || y < 0 || y >= _height) {
    return; // 超出显示区域
  }

  uint16_t index = x + (y / 8) * _width;
  ESP_LOGW(TAG, "Setting pixel at (%d, %d), index: %d", x, y,
           index); // 打印索引，调试用

  if (color) {
    _buffer[index] |= (1 << (y % 8)); // 设置像素
  } else {
    _buffer[index] &= ~(1 << (y % 8)); // 清除像素
  }
}

void SSD1306::display_image(int page, int seg, uint8_t *images,int width) {
  if (page >= _pages)
    return;
  if (seg >= _width)
    return;

  int _seg = seg + CONFIG_OFFSETX;
  uint8_t columLow = _seg & 0x0F;
  uint8_t columHigh = (_seg >> 4) & 0x0F;

  int _page = page;
  if (_flip) {
    _page = (_pages - page) - 1;
  }

  uint8_t *out_buf;
  out_buf = (uint8_t *)malloc(width < 4 ? 4 : width + 1);
  if (out_buf == NULL) {
    ESP_LOGE(TAG, "malloc fail");
    return;
  }
  int out_index = 0;
  out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
  // Set Lower Column Start Address for Page Addressing Mode
  out_buf[out_index++] = (0x00 + columLow);
  // Set Higher Column Start Address for Page Addressing Mode
  out_buf[out_index++] = (0x10 + columHigh);
  // Set Page Start Address for Page Addressing Mode
  out_buf[out_index++] = 0xB0 | _page;

  esp_err_t res;
  res = i2c_master_transmit(dev_handle, out_buf, out_index, I2C_TICKS_TO_WAIT);
  if (res != ESP_OK)
    ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)",
             I2C_ADDRESS, I2C_NUM, res, esp_err_to_name(res));

  out_buf[0] = OLED_CONTROL_BYTE_DATA_STREAM;
  memcpy(&out_buf[1], images, width);

  res = i2c_master_transmit(dev_handle, out_buf, width + 1, I2C_TICKS_TO_WAIT);
  if (res != ESP_OK)
    ESP_LOGE(TAG, "Could not write to device [0x%02x at %d]: %d (%s)",
             I2C_ADDRESS, I2C_NUM, res, esp_err_to_name(res));
  free(out_buf);
}

void SSD1306::flush() {
  ESP_LOGW(TAG, "flush oled");
  for (uint8_t page = 0; page < _pages; page++) {
    // 设置当前页面地址
    writeCommand(0xB0 + page); // 页地址，0xB0 + 页号
    writeCommand(0x00);        // 列地址低 4 位
    writeCommand(0x10);        // 列地址高 4 位

    // 向显示器写入当前页的数据
    i2c_master_transmit(dev_handle, _buffer + (page * _width), _width,
                        1000 / portTICK_PERIOD_MS);
  }
}

void SSD1306::probe() {
  esp_err_t res;
  int foundCount = 0; for (int i = 0; i < 128; i++) {
    res = i2c_master_probe(bus_handle, i, 50);
    if (res == ESP_OK) {
      printf("+++ %.2x\n", i);
      foundCount++;
    }
  }
}

esp_err_t SSD1306::probeSSD1306() {
  int count = 10;
  esp_err_t err;
  while (count) {
    err = i2c_master_probe(bus_handle, I2C_ADDRESS, 50);

    if (err == ESP_OK) {
      return err;
    }

    count--;
  }

  return err;
}