#include "driver/i2c_master.h"
#include <stdint.h>

#define OLED_CONTROL_BYTE_CMD_SINGLE 0x80
#define OLED_CONTROL_BYTE_CMD_STREAM 0x00
#define OLED_CONTROL_BYTE_DATA_SINGLE 0xC0
#define OLED_CONTROL_BYTE_DATA_STREAM 0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST 0x81 // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM 0xA4
#define OLED_CMD_DISPLAY_ALLON 0xA5
#define OLED_CMD_DISPLAY_NORMAL 0xA6
#define OLED_CMD_DISPLAY_INVERTED 0xA7
#define OLED_CMD_DISPLAY_OFF 0xAE // 关闭显示
#define OLED_CMD_DISPLAY_ON 0xAF  // 打开显示

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE 0x20
#define OLED_CMD_SET_HORI_ADDR_MODE 0x00 // Horizontal Addressing Mode
#define OLED_CMD_SET_VERT_ADDR_MODE 0x01 // Vertical Addressing Mode
#define OLED_CMD_SET_PAGE_ADDR_MODE 0x02 // Page Addressing Mode
#define OLED_CMD_SET_COLUMN_RANGE                                              \
  0x21 // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F =
       // COL127
#define OLED_CMD_SET_PAGE_RANGE                                                \
  0x22 // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP_0 0xA0
#define OLED_CMD_SET_SEGMENT_REMAP_1 0xA1
#define OLED_CMD_SET_MUX_RATIO 0xA8 // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE 0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET 0xD3 // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP 0xDA    // follow with 0x12
#define OLED_CMD_NOP 0xE3                // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV 0xD5 // follow with 0x80
#define OLED_CMD_SET_PRECHARGE 0xD9       // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT 0xDB   // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP 0x8D // follow with 0x14

// Scrolling Command
#define OLED_CMD_HORIZONTAL_RIGHT 0x26
#define OLED_CMD_HORIZONTAL_LEFT 0x27
#define OLED_CMD_CONTINUOUS_SCROLL 0x29
#define OLED_CMD_DEACTIVE_SCROLL 0x2E
#define OLED_CMD_ACTIVE_SCROLL 0x2F
#define OLED_CMD_VERTICAL 0xA3

#define I2C_ADDRESS 0x3C
#define SPI_ADDRESS 0xFF

#define CONFIG_OFFSETX 0

class SSD1306 {
private:
  SSD1306();
  ~SSD1306();

  i2c_master_bus_handle_t bus_handle;
  i2c_master_dev_handle_t dev_handle;

  uint16_t _width = 128;
  uint16_t _height = 64;
  uint16_t _flip = 0;
  uint16_t _pages = 8; // 64高度是8,32高度是4

  int _bufferSize = 128 + 1;
  uint8_t _buffer[128 + 1] = {
      0x00,
  };

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

  void display_image(int page, int seg, uint8_t *images, int width);

  void probe();

  esp_err_t probeSSD1306();
};
