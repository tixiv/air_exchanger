
#include "driver/uart.h"


#define DE_RE_PIN 18  // Direction control
#define BAUD_RATE 9600  // Match device spec

void rs485_uart_init() {
  uart_config_t uart_config = {
    .baud_rate = 9600,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,  // Default threshold
    .source_clk = UART_SCLK_DEFAULT,  // Add clock source
    .flags = {  // Nested struct initialization
        .allow_pd = false,  // Disallow power-down during sleep
        .backup_before_sleep = false  // Explicit initialization
    }
  };

  ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, 1024, 0, 0, NULL, 0));
  
  ESP_ERROR_CHECK(uart_param_config(UART_NUM_2, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(UART_NUM_2, 17, 16, 18, UART_PIN_NO_CHANGE));
  ESP_ERROR_CHECK(uart_set_mode(UART_NUM_2, UART_MODE_RS485_HALF_DUPLEX));

}

void uart_putc(uint8_t c)
{
  uart_write_bytes(UART_NUM_2, &c, 1);
}

uint8_t uart_getc_nb(uint8_t *c)
{
  return uart_read_bytes(UART_NUM_2, c, 1, 0);
}
