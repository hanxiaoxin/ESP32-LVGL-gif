#include "driver/uart.h"
#include <driver/gpio.h>
#include <stdint.h>

#define UART_TX GPIO_NUM_6
#define UART_RX GPIO_NUM_7

#define UART_PORT_NUM UART_NUM_1
#define UART_BAUD_RATE 115200
#define UART_BUFFER_SIZE 1024

void init_uart();
void uart_send_data(const char *data);