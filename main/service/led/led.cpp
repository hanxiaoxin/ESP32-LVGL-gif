#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#define LED_GPIO GPIO_NUM_8

#define TAG "LED"

void turn_on() {
  gpio_set_level(LED_GPIO, 0);
  gpio_set_level(GPIO_NUM_0, 1);
}

void turn_off() {
  gpio_set_level(LED_GPIO, 1);
  gpio_set_level(GPIO_NUM_0, 0);
}

void led_init(void *params){
  // 配置 GPIO18 为输出模式
  gpio_config_t led_conf = {.pin_bit_mask = (1ULL << LED_GPIO),
                            .mode = GPIO_MODE_OUTPUT,
                            .pull_up_en = GPIO_PULLUP_DISABLE,
                            .pull_down_en = GPIO_PULLDOWN_DISABLE,
                            .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&led_conf);

  gpio_config_t pin0_conf = {.pin_bit_mask = (1ULL << GPIO_NUM_0),
                             .mode = GPIO_MODE_OUTPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&pin0_conf);

  gpio_config_t pin1_conf = {.pin_bit_mask = (1ULL << GPIO_NUM_1),
                             .mode = GPIO_MODE_OUTPUT,
                             .pull_up_en = GPIO_PULLUP_DISABLE,
                             .pull_down_en = GPIO_PULLDOWN_DISABLE,
                             .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&pin1_conf);
  gpio_set_level(GPIO_NUM_1, 0);

  while(true) {
    turn_on();
    ESP_LOGI(TAG, "LED turned on");
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    turn_off();
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}