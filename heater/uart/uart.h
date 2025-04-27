#ifndef UART_H
#define UART_H

/**
 * UART Library
 *
 * #define F_CPU 16000000         // Oszillator-Frequenz in Hz
 * #define UART_INTERRUPT 1
 * #define UART_BAUD_RATE 19200
 * #define UART_RXBUFSIZE 16
 * #define UART_TXBUFSIZE 16
 * #define UART_LINE_BUFFER_SIZE 40
 * #define UART_LEDS             // PC0 and PC1 toggle on tx and rx interrupt
 *
 */

//read config
#include "../config.h"

#include <inttypes.h>
#include <avr/pgmspace.h>

//prototypes
void uart_init(void);

void uart_putc(uint8_t c);
void uart_putstr(char *str);
void uart_putstr_P(PGM_P str);

uint8_t uart_getc(void);
char uart_getc_nb(uint8_t *c);		// non blocking, returns 1 on success

//prototypes
void uart_0_init(void);

void uart_0_putc(uint8_t c);
void uart_0_putstr(char *str);
void uart_0_putstr_P(PGM_P str);

uint8_t uart_0_getc(void);
char uart_0_getc_nb(uint8_t *c);		// non blocking, returns 1 on success

//prototypes
void uart_1_init(void);

void uart_1_putc(uint16_t c);
void uart_1_putstr(char *str);
void uart_1_putstr_P(PGM_P str);

uint16_t uart_1_getc(void);
char uart_1_getc_nb(uint16_t *c);		// non blocking, returns 1 on success

//prototypes
void uart_2_init(void);

void uart_2_putc(char c);
void uart_2_putstr(char *str);
void uart_2_putstr_P(PGM_P str);

char uart_2_getc(void);
char uart_2_getc_nb(char *c);		// non blocking, returns 1 on success

//prototypes
void uart_3_init(void);

void uart_3_putc(char c);
void uart_3_putstr(char *str);
void uart_3_putstr_P(PGM_P str);

char uart_3_getc(void);
char uart_3_getc_nb(char *c);		// non blocking, returns 1 on success


#ifdef UART_HEXDUMP
	//hexdump utility
	void uart_hexdump(uint8_t *buf, int len);
#endif

#ifdef UART_GETLINE
	//get one Cariage return terminated line
	//echo charakters back on Uart
	//returns buffer with zero terminated line on success, 0 pointer otherwise
	char *uart_getline(void);
#endif
char uart_rxbuffer_notempty(void);	//returns 1 if queue is not empty, otherwise 0
#if UART_RXBUFSIZE < 255
uint8_t uart_getrxqueuelength(void); //returns queue length
#else
uint16_t uart_getrxqueuelength(void); //returns queue length
#endif
#endif
