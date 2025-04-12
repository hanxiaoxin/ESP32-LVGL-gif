#include "ssd1306.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>
#include <string.h>

#define I2C_SCL_NUM GPIO_NUM_4
#define I2C_SDA_NUM GPIO_NUM_3

#define TAG "SSD1306"
#include "driver/i2c_master.h"
#include "esp_log.h"

SSD1306::SSD1306() {
  i2c_master_bus_config_t i2c_mst_config = {
      .i2c_port = I2C_NUM_0,
      .sda_io_num = I2C_SDA_NUM,
      .scl_io_num = I2C_SCL_NUM,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags = {.enable_internal_pullup = true, .allow_pd = false}};
  ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_mst_config, &bus_handle));

  i2c_device_config_t dev_cfg = {.dev_addr_length = I2C_ADDR_BIT_LEN_7,
                                 .device_address = i2c_address,
                                 .scl_speed_hz = 400000,
                                 .scl_wait_us = 0,
                                 .flags = {.disable_ack_check = false}};

  ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle));
}

SSD1306::~SSD1306() {}

void SSD1306::writeCommand(uint8_t command) {
  uint8_t cmd[2] = {0x00, command}; // 前面加上 0x00 表示命令模式
  // ESP_LOGW(TAG, "write command: %d", command);
  i2c_master_transmit(dev_handle, cmd, 2, 200 / portTICK_PERIOD_MS);
}

void SSD1306::init() {
  // 复位 SSD1306 显示器
  writeCommand(0xAE); // 关闭显示
  writeCommand(0xD5); // 设置时钟分频因子
  writeCommand(0x80); // 设置时钟分频值
  writeCommand(0xA8); // 设置多路复用比率
  writeCommand(0x3F); // 设置为 64（取决于屏幕的高度）
  writeCommand(0xD3); // 设置显示偏移
  writeCommand(0x00); // 偏移值
  writeCommand(0x40); // 设置起始行地址
  writeCommand(0x8D); // 充电泵设置
  writeCommand(0x14); // 启用充电泵
  writeCommand(0x20); // 设置内存寻址模式
  writeCommand(0x00); // 水平寻址模式
  writeCommand(0xA1); // 设置列扫描方向
  writeCommand(0xC8); // 设置行扫描方向
  writeCommand(0xDA); // 设置 COM 引脚硬件配置
  writeCommand(0x12); // 设置 COM 引脚硬件配置
  writeCommand(0x81); // 设置对比度控制
  writeCommand(0x7F); // 设置对比度
  writeCommand(0xA4); // 输出正常显示
  writeCommand(0xA6); // 设置正常显示（不反转）
  writeCommand(0xAF); // 打开显示
}

void SSD1306::full() {
  memset(buffer + 1, 0XFF, bufferSize - 1); // 全白
  flush();
}

void SSD1306::clear() {
  memset(buffer + 0, 0, bufferSize - 1);
  flush();
}

void SSD1306::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x < 0 || x >= width || y < 0 || y >= height) {
    return; // 超出显示区域
  }

  uint16_t index = x + (y / 8) * width;
  ESP_LOGW(TAG, "Setting pixel at (%d, %d), index: %d", x, y,
           index); // 打印索引，调试用

  if (color) {
    buffer[index] |= (1 << (y % 8)); // 设置像素
  } else {
    buffer[index] &= ~(1 << (y % 8)); // 清除像素
  }
}

void SSD1306::flush() {
  ESP_LOGW(TAG, "flush oled");
  for (uint8_t page = 0; page < page_count; page++) {
    // 设置当前页面地址
    writeCommand(0xB0 + page); // 页地址，0xB0 + 页号
    writeCommand(0x00);        // 列地址低 4 位
    writeCommand(0x10);        // 列地址高 4 位

    // 向显示器写入当前页的数据
    i2c_master_transmit(dev_handle, buffer + (page * width), width,
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
    err = i2c_master_probe(bus_handle, i2c_address, 50);

    if (err == ESP_OK) {
      return err;
    }

    count--;
  }

  return err;
}