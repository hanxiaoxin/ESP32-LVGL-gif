#include "driver/rmt_tx.h"
#include "led_strip.h"

#define LED_STRIP_RMT_CHANNEL 0
#define LED_STRIP_GPIO 1

led_strip_handle_t led_strip;

void ws2812_init() {
  // 配置RMT驱动
  led_strip_config_t strip_config = {
      .strip_gpio_num = LED_STRIP_GPIO,
      .max_leds = 1,
      .led_model = LED_MODEL_WS2812,
      .color_component_format =
          LED_STRIP_COLOR_COMPONENT_FMT_RGB, // WS2812通常是GRB
      .flags =
          {
              .invert_out = 0, // 不反转输出
          },
  };

  led_strip_rmt_config_t rmt_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = 10 * 1000 * 1000, // 10MHz -> 0.1us精度
      .mem_block_symbols = 64,           // 内存块大小，通常够用
      .flags =
          {
              .with_dma = 0, // 使用DMA
          },
  };

  led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);

  // 设置颜色，比如点亮第一个灯，红色
  led_strip_set_pixel(led_strip, 0, 255, 0, 0);
  led_strip_refresh(led_strip);                 // 刷新，把数据发出去

  // 延时
  vTaskDelay(pdMS_TO_TICKS(1000));

  // 设置颜色，
  led_strip_set_pixel(led_strip, 0, 0, 255, 0); 
  led_strip_refresh(led_strip);

  // 延时
  vTaskDelay(pdMS_TO_TICKS(1000));

  // 设置颜色
  led_strip_set_pixel(led_strip, 0, 255, 0, 255);
  led_strip_refresh(led_strip);

  // 延时
  vTaskDelay(pdMS_TO_TICKS(1000));

  // 关灯
  led_strip_clear(led_strip);
}