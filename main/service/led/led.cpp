#include "led_pwm.h"
#include "led_gpio.h"

void led_blink(void *param) { 
    // pwm_blink(param);
    gpio_led_blink();
 }