#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "config.h"
#include "freertos/event_groups.h"

#define TAG "PWM"

#define LEDC_LS_CH0_CHANNEL LEDC_CHANNEL_0
#define LEDC_LS_TIMER LEDC_TIMER_0
#define LEDC_LS_MODE LEDC_LOW_SPEED_MODE

#define LEDC_TEST_CH_NUM (1)
#define LEDC_TEST_DUTY (4096)
#define LEDC_TEST_FADE_TIME (2'000)

#define LEDC_FADE_END_BIT (1 << 0)
static EventGroupHandle_t ledc_event_group;

static IRAM_ATTR bool cb_ledc_fade_end_event(const ledc_cb_param_t *param,
                                             void *user_arg) {
  BaseType_t taskAwoken = pdFALSE;

  // 增加空指针检查
  if (!param) {
    ESP_DRAM_LOGE(TAG, "Null param received in ISR");
    return pdFALSE;
  }

  if (param->event == LEDC_FADE_END_EVT) {
    // 使用更安全的位定义
    const EventBits_t target_bit = (EventBits_t)(1ULL << 0);

    // 增加事件组有效性检查
    if (ledc_event_group != NULL) {
      BaseType_t result =
          xEventGroupSetBitsFromISR(ledc_event_group, target_bit, &taskAwoken);

      if (result != pdPASS) {
        ESP_DRAM_LOGW(TAG, "Event group set bits failed: %d", result);
      }
    }
  } else {
    ESP_DRAM_LOGW(TAG, "Unhandled event: %d", param->event);
  }

  return (taskAwoken == pdTRUE);
}

ledc_channel_config_t ledc_channel;

void pwm_led_init() {
  // 创建事件组
  ledc_event_group = xEventGroupCreate();

  ledc_timer_config_t ledc_timer = {
      .speed_mode = LEDC_LS_MODE,           // timer mode
      .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
      .timer_num = LEDC_LS_TIMER,           // timer index
      .freq_hz = 4000,            // frequency of PWM signal
      .clk_cfg = LEDC_AUTO_CLK,             // Auto select the source clock
      .deconfigure = false};

  ledc_timer_config(&ledc_timer);

  ledc_channel_config_t ledc_channel = {
      .gpio_num = LED_GPIO,                         // GPIO number
      .speed_mode = LEDC_LS_MODE,                   // timer mode
      .channel = LEDC_LS_CH0_CHANNEL,               // channel index
      .intr_type = LEDC_INTR_FADE_END,              // interrupt type
      .timer_sel = LEDC_LS_TIMER,                   // timer index
      .duty = 0,                                    // duty cycle
      .hpoint = 0,                                  // hpoint value
      .sleep_mode = LEDC_SLEEP_MODE_NO_ALIVE_NO_PD, // sleep mode
      .flags = {
          .output_invert = 0,
      }};

  ledc_channel_config(&ledc_channel);

  ledc_fade_func_install(0);
  ledc_cbs_t callbacks = {.fade_cb = cb_ledc_fade_end_event};
  ledc_cb_register(ledc_channel.speed_mode, ledc_channel.channel, &callbacks,
                   NULL);
}

// 定义回调函数
void fade_callback(ledc_channel_t channel) {
  static int direction = 1; // 1=渐亮，-1=渐暗
  ESP_LOGW(TAG, "Fade callback called, direction: %d", direction);
  if (direction) {
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, channel, 4096,
                            LEDC_TEST_FADE_TIME);
    direction = 0; // 反转方向
  } else {
    ledc_set_fade_with_time(LEDC_LOW_SPEED_MODE, channel, 0,
                            LEDC_TEST_FADE_TIME);
    direction = 1; // 反转方向
  }
}

void pwm_blink(void *params) {
  while(1){
    ESP_LOGI(TAG, "LED PWM fade start...");
    fade_callback(LEDC_CHANNEL_0); // 调用回调函数开始渐变
    ledc_fade_start(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT);
    ESP_LOGI(TAG, "LED PWM fade end...");
  }
}