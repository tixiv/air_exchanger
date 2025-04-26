
#include "rs485_com.h"
#include <string.h>

static RS485_Buffer_t rx_buff;

static uint8_t rx_index;

RS485_Buffer_t *update_rs485_com(void)
{
    uint8_t c;
    if (!uart_getc_nb(&c))
        return 0;

    if (!(rx_index < sizeof(rx_buff)))
    {
        rx_index = 0;
        return 0;
    }

    rx_buff.bytes[rx_index] = c;

    if (rx_index == 0 && c != 0x55)
    {
        return 0;
    }

    if (rx_index == 1 && c != 0xaa)
    {
        rx_index = 0;
        return 0;
    }

    if (rx_index > 3)
    {
        uint8_t length = rx_buff.extra_len + 5;

        if (rx_index >= length-1)
        {
            // done
            rx_index = 0;
            return &rx_buff;
        }
    }

    rx_index ++;
    return 0;
}

void rs485_transmit(uint8_t address, uint8_t command, uint8_t *extra_data, uint8_t extra_len)
{
    static RS485_Buffer_t tx_buff = {.bytes = {0x55, 0xaa}};

    tx_buff.address = address;
    tx_buff.command = command;
    tx_buff.extra_len = extra_len;
    if (extra_len)
    {
        memcpy(tx_buff.data, extra_data, extra_len);
    }

    uint8_t len = extra_len + 5;

    for (uint8_t i=0; i<len; i++)
    {
        uart_putc(tx_buff.bytes[i]);
    }
}
