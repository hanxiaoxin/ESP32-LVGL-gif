#include "driver/gptimer.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <stdio.h>

static const char *TAG = "hardware_timer";
static uint32_t period = 1000 * 1000; // 1s

static bool IRAM_ATTR on_alarm_callback(gptimer_handle_t timer,
                                        const gptimer_alarm_event_data_t *edata,
                                        void *user_ctx) {
  uint32_t *period = (uint32_t *)user_ctx;
  // 注意：这个函数是在中断上下文中执行的，不能调用阻塞/内存分配操作
  ESP_DRAM_LOGI(TAG, "Timer alarm! Count value: %llu, %u", edata->count_value,
                *period);
  return true; // 返回 true 表示继续调用该回调
}

void hardware_start() {
  gptimer_handle_t gptimer = NULL;
  gptimer_config_t timer_config = {
      .clk_src = GPTIMER_CLK_SRC_DEFAULT,
      .direction = GPTIMER_COUNT_UP,
      .resolution_hz = period, // 1MHz, 1 tick = 1us
      .intr_priority = 0,
      .flags = {0,0,0},
  };
  ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));

  // 注册回调
  gptimer_event_callbacks_t cbs = {
      .on_alarm = on_alarm_callback,
  };
  ESP_ERROR_CHECK(gptimer_register_event_callbacks(
      gptimer, &cbs, NULL));

  gptimer_alarm_config_t alarm_config = {
      .alarm_count = 1000000,
      .reload_count = 0, 
      .flags =
          {
              .auto_reload_on_alarm = true, // 初始化 flags 中的 auto_reload_on_alarm 字段
          },
  };
  ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));

  ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cbs, &period));
  ESP_ERROR_CHECK(gptimer_enable(gptimer));
  ESP_ERROR_CHECK(gptimer_start(gptimer));

  ESP_LOGI(TAG, "gptimer started!");
}