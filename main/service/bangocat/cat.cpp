#include "bob_data.h"
#include "display/ssd1306.h"
#include "esp_log.h"
#include <esp_err.h>
#include <freertos/FreeRTOS.h>

void showBangoCat(SSD1306 &display) {
  DisplayData dd;

  display.init();
  display.invert(0);
  display.clear();

  while(1) {
    for (int i = 0; i < allArray_LEN; ++i) {
      memcpy(dd.Data, allArray[i], sizeof(dd.Data));
      for (int page = 0; page < 8; page++) {
        display.display_image(page, 0, &dd.Data[page * 128], 128);
      }
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }
  }
}