#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include "board/button.h"
#include "background_task.h"

#define BOOT_BUTTON_GPIO GPIO_NUM_9
#define TOUCH_BUTTON_GPIO GPIO_NUM_10

class Application {
public:
  Button boot_button_;
  Button touch_button_;
  BackgroundTask *background_task_ = nullptr;

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

  int clock_ticks_ = 0;

  void OnClockTimer();

  EventGroupHandle_t event_group_ = nullptr;
  esp_timer_handle_t clock_timer_handle_ = nullptr;
};

#endif // _APPLICATION_H_
