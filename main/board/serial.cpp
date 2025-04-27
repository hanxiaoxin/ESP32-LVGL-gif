#include "serial.h"
#include "driver/uart.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "UART";

void uart_send_data(const char *data) {
  int len = strlen((char *)data);
  uart_write_bytes(UART_PORT_NUM, (const char *)data, len);
}

void init_uart() {
  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = UART_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .source_clk = UART_SCLK_DEFAULT,
      .flags = {
          .allow_pd = 0,
          .backup_before_sleep = 0,
      },
  };
  int intr_alloc_flags = 0;

  ESP_ERROR_CHECK(uart_driver_install(UART_PORT_NUM, UART_BUFFER_SIZE * 2, 0, 0,
                                      NULL, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT_NUM, UART_TX, UART_RX,
                               UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
  /*
    uint8_t *data = (uint8_t *)"hello world12345\r\n";

     while (1) {
      int len = strlen((char *)data); // 注意：需要有长度，否则 len 可能是随机值
      uart_write_bytes(UART_PORT_NUM, (const char *)data, len);
      if (len) {
        ESP_LOGI(TAG, "Recv str: %s", (char *)data);
      }
      vTaskDelay(pdMS_TO_TICKS(300)); // 加点延迟，否则太快了
    } */
}