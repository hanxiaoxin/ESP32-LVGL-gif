#include "driver/i2c_master.h"
#include <stdint.h>

class SSD1306 {
private:
  SSD1306();
  ~SSD1306();

  i2c_master_bus_handle_t bus_handle;
  i2c_master_dev_handle_t dev_handle;

  uint16_t i2c_address = 0x3c; // Default I2C address for SSD1306
  uint16_t width = 128;
  uint16_t height = 64;
  uint16_t page_count = 8;
  int bufferSize = 1025;
  uint8_t buffer[1025] = {0x00,};

public:
  static SSD1306 &GetInstance() {
    static SSD1306 instance;
    return instance;
  }

  // 删除拷贝构造函数和赋值运算符
  SSD1306(const SSD1306 &) = delete;
  SSD1306 &operator=(const SSD1306 &) = delete;

  void writeCommand(uint8_t command);

  void full();

  void flush();

  void clear();

  void init();

  void drawPixel(int16_t x, int16_t y, uint16_t color);

  void probe();

  esp_err_t probeSSD1306();
};
