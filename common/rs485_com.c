
#include "rs485_com.h"
#include <string.h>
#include <stdbool.h>

static RS485_Buffer_t rx_buff;
static RS485_Buffer_t tx_buff = {.bytes = {0x55, 0xaa}};

static uint8_t rx_index;

static bool scheduled_reply;

static void rs485_transmit_tx_buff(void)
{
    uint8_t len = tx_buff.extra_len + 5;

    for (uint8_t i=0; i<len; i++)
    {
        uart_putc(tx_buff.bytes[i]);
    }
}

RS485_Buffer_t *update_rs485_com(void)
{
    if (scheduled_reply)
    {
        scheduled_reply = false;
        rs485_transmit_tx_buff();
    }

    uint8_t c;
    while(uart_getc_nb(&c))
    {
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
    }
    
    return 0;
}

static void rs485_init_tx_buff(uint8_t address, uint8_t command, void *extra_data, uint8_t extra_len)
{
    tx_buff.address = address;
    tx_buff.command = command;
    tx_buff.extra_len = extra_len;
    if (extra_len && extra_data)
    {
        memcpy(tx_buff.data, extra_data, extra_len);
    }
}

void rs485_transmit(uint8_t address, uint8_t command, void *extra_data, uint8_t extra_len)
{
    rs485_init_tx_buff(address, command, extra_data, extra_len);
    rs485_transmit_tx_buff();
}

void rs485_schedule_reply(uint8_t address, uint8_t command, void *extra_data, uint8_t extra_len)
{
    rs485_init_tx_buff(address, command, extra_data, extra_len);
    scheduled_reply = true;
}
