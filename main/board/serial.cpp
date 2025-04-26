#include "serial.h"
#include "driver/uart.h"
#include "esp_log.h"

static const char *TAG = "UART";
static const uart_port_t uart_num = UART_PORT;

void init_uart() {
  uart_config_t uart_config = {
      .baud_rate = UART_BAUD_RATE,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
      .source_clk = UART_SCLK_APB,
      .flags =
          {
              .allow_pd = 0,
              .backup_before_sleep = 0,
          },
  };
  // Configure UART parameters
  ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(uart_num, UART_TX, UART_RX, UART_PIN_NO_CHANGE,
                               UART_PIN_NO_CHANGE));

  // Setup UART buffered IO with event queue
  const int uart_buffer_size = UART_BUFFER_SIZE;
  QueueHandle_t uart_queue;
  // Install UART driver using an event queue here
  ESP_ERROR_CHECK(uart_driver_install(uart_num, uart_buffer_size,
                                      uart_buffer_size, 10, &uart_queue, 0));
  ESP_LOGI(TAG, "UART driver installed");
}

void send_data(const char *data, size_t len) {
  if (data == NULL || len == 0) {
    ESP_LOGE(TAG, "Invalid data to send");
    return;
  }

  // Write data to the UART port
  uart_write_bytes(uart_num, data, len);
  ESP_LOGI(TAG, "Sent data: %s", data);
}