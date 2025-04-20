#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "config.h"

#define TAG "LED"

void gpio_led_turn_on() {
  gpio_set_level(LED_GPIO, 0);
  gpio_set_level(GPIO_NUM_0, 1);
}

void gpio_led_turn_off() {
  gpio_set_level(LED_GPIO, 1);
  gpio_set_level(GPIO_NUM_0, 0);
}

void gpio_led_blink(void *params) {
  while(true){
    gpio_led_turn_on();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_led_turn_off();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void gpio_led_init(){
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
}