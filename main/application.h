#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include "background_task.h"

enum DeviceState {
  kDeviceStateUnknown,
  kDeviceStateStarting,
  kDeviceStateWifiConfiguring,
  kDeviceStateIdle,
  kDeviceStateConnecting,
  kDeviceStateListening,
  kDeviceStateSpeaking,
  kDeviceStateUpgrading,
  kDeviceStateActivating,
  kDeviceStateFatalError
};

class Application {
public:
  BackgroundTask *background_task_ = nullptr;

  static Application &GetInstance() {
    static Application instance;
    return instance;
  }
  // 删除拷贝构造函数和赋值运算符
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  void Start();
  DeviceState GetDeviceState() const { return device_state_; }
  void SetDeviceState(DeviceState state);

private:
  Application();
  ~Application();

  int clock_ticks_ = 0;

  void OnClockTimer();

  EventGroupHandle_t event_group_ = nullptr;
  esp_timer_handle_t clock_timer_handle_ = nullptr;
  volatile DeviceState device_state_ = kDeviceStateUnknown;
};

#endif // _APPLICATION_H_
