#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include "board/button.h"

#define BOOT_BUTTON_GPIO GPIO_NUM_9
#define TOUCH_BUTTON_GPIO GPIO_NUM_10

class Application {
public:
  Button boot_button_;
  Button touch_button_;

  static Application &GetInstance() {
    static Application instance;
    return instance;
  }
  // 删除拷贝构造函数和赋值运算符
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  void Start();
  void initButtonEvents();

private:
  Application();
  ~Application();
};

#endif // _APPLICATION_H_
