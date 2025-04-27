
#include <stdint.h>

typedef union {
    struct {
        uint8_t magic1;
        uint8_t magic2;
        uint8_t address;
        uint8_t command;
        uint8_t extra_len;
        uint8_t data[];
    };
    uint8_t bytes[32];
} RS485_Buffer_t;

RS485_Buffer_t *update_rs485_com(void);

void rs485_transmit(uint8_t address, uint8_t command, void *extra_data, uint8_t extra_data_len);
