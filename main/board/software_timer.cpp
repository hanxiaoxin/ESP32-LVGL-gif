#include "esp_timer.h"
#include <stdio.h>

void soft_start(void (*periodic_timer_callback)(void *arg), int *period) {
  printf("Soft timer start, %d\n", *period);
  esp_timer_handle_t periodic_timer;
  esp_timer_create_args_t timer_args = {
      .callback = periodic_timer_callback,
      .arg = (void *)period,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "periodic_timer",
      .skip_unhandled_events = false,
  };

  // 创建定时器
  ESP_ERROR_CHECK(esp_timer_create(&timer_args, &periodic_timer));

  // 设定每 1000 毫秒
  ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, *period));
}