#include "board/ntp.h"
#include "display/ssd1306.h"
#include "esp_log.h"
#include <esp_err.h>
#include <freertos/FreeRTOS.h>
#include <sys/time.h>
#include <time.h>
#include "board/serial.h"

static const char *TAG = "CLOCK";

void showClock(SSD1306 &display) {
  display.init();
  display.clear();

  time_t now;

  while (1) {
    time(&now);
    localtime_r(&now, &timeinfo);
    char date_str[32];
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", &timeinfo);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);

    display.display_text(0, date_str, strlen(date_str));
    display.display_text(1, time_str, strlen(time_str));
    ESP_LOGI(TAG, "Show current date/time is: %s %s", date_str, time_str);

    char buf[96];
    snprintf(buf, sizeof(buf), "Date: %s Time: %s\n", date_str, time_str);
    uart_send_data(buf);

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}