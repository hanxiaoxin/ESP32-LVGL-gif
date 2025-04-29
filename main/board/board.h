#ifndef BOARD_H
#define BOARD_H

#include "button.h"
#include "display/display.h"
#include "driver/i2c_master.h"
#include <esp_lcd_panel_ops.h>
#include <esp_lcd_panel_vendor.h>
#include <string>

class Display;
class Board {
private:
  Board(const Board &) = delete;            // 禁用拷贝构造函数
  Board &operator=(const Board &) = delete; // 禁用赋值操作
  Button boot_button_;
  Button touch_button_;

  i2c_master_bus_handle_t display_i2c_bus_;
  esp_lcd_panel_io_handle_t panel_io_ = nullptr;
  esp_lcd_panel_handle_t panel_ = nullptr;

  void initButtonEvents();
  void initOledDisplay();
  void initLcdDisplay();

protected:
  Board();
  std::string GenerateUuid();

  // 软件生成的设备唯一标识
  std::string uuid_;

public:
  std::string board_name = "HANXIAOXIN-ESP32C3-OLED"; // 板子名称
  std::string board_type = "ESP32C3";                 // 板子类型
  Display *display_ = new NoDisplay();

  static Board &GetInstance() {
    static Board instance;
    return instance;
  }

  ~Board() = default;
  std::string GetBoardType();
  std::string GetUuid() { return uuid_; }
  Display *GetDisplay();
  std::string GetJSON();
  std::string GetBoardJson();
  const char *GetNetworkStateIcon();
  esp_err_t probe_SSD1306();
};
#endif // BOARD_H