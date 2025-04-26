/* USART-Init beim ATmegaXX */
/* modified by siro 25.4.2010 to add atmega48 support */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "uart.h"

#if defined(__AVR_ATmega1280__)
	#if defined(USE_UART_1)
		#define UCSRB UCSR1B
		#define UCSRC UCSR1C
		#define UDR UDR1
		#define UBRRH UBRR1H
		#define UBRRL UBRR1L
		#define TXEN TXEN1
		#define RXEN RXEN1
		#define UCSRA UCSR1A
		#define UDRIE UDRIE1
		#define UCSZ0 UCSZ10
		#define UCSZ1 UCSZ11
		#define UCSZ2 UCSZ12
		#define RXCIE RXCIE1
		#define TXCIE TXCIE1
		#define RXC RXC1
		#define RXB8 RXB81
		#define TXB8 TXB81
		#define UDRE UDRE1
		#define UART_UDRE_VECTOR USART1_UDRE_vect
		#define UART_RECV_VECTOR USART1_RX_vect
		#define uart_init               uart_1_init
		#define uart_getc_nb            uart_1_getc_nb
		#define uart_getc               uart_1_getc
		#define uart_putc               uart_1_putc
		#define uart_putstr             uart_1_putstr
		#define uart_putstr_P           uart_1_putstr_P
		#define uart_getrxqueuelength   uart_1_getrxqueuelength
		#define uart_rxbuffer_notempty  uart_1_rxbuffer_notempty
		#define uart_getline            uart_1_getline
		#define UART_BAUD_RATE          UART_BAUD_RATE_1
	#else
		#define UCSRB UCSR0B
		#define UCSRC UCSR0C
		#define UDR UDR0
		#define UBRRH UBRR0H
		#define UBRRL UBRR0L
		#define TXEN TXEN0
		#define RXEN RXEN0
		#define UCSRA UCSR0A
		#define UDRIE UDRIE0
		#define UCSZ0 UCSZ00
		#define UCSZ1 UCSZ01
		#define UCSZ2 UCSZ02
		#define RXCIE RXCIE0
		#define TXCIE TXCIE0
		#define RXC RXC0
		#define RXB8 RXB80
		#define TXB8 TXB80
		#define UDRE UDRE0
		#define UART_UDRE_VECTOR USART0_UDRE_vect
		#define UART_RECV_VECTOR USART0_RX_vect
		#define uart_init               uart_0_init
		#define uart_getc_nb            uart_0_getc_nb
		#define uart_getc               uart_0_getc
		#define uart_putc               uart_0_putc
		#define uart_putstr	            uart_0_putstr
		#define uart_putstr_P           uart_0_putstr_P
		#define uart_getrxqueuelength   uart_0_getrxqueuelength
		#define uart_rxbuffer_notempty  uart_0_rxbuffer_notempty
		#define uart_getline            uart_0_getline
		#define UART_BAUD_RATE          UART_BAUD_RATE_0
	#endif
#elif defined(__AVR_ATmega128__)
	#ifndef USE_UART_1
		//UART 0 is default
		#define UCSRA UCSR0A
		#define UCSRB UCSR0B
		#define UCSRC UCSR0C
		#define UDR UDR0
		#define UBRRH UBRR0H
		#define UBRRL UBRR0L
		#define URSEL UMSEL
		#define UART_UDRE_VECTOR USART0_UDRE_vect
		#define UART_RECV_VECTOR USART0_RX_vect
	#else
		//UART 1
		#define UCSRA UCSR1A
		#define UCSRB UCSR1B
		#define UCSRC UCSR1C
		#define UDR UDR1
		#define UBRRH UBRR1H
		#define UBRRL UBRR1L
		#define URSEL UMSEL
		#define UART_UDRE_VECTOR USART1_UDRE_vect
		#define UART_RECV_VECTOR USART1_RX_vect
	#endif
#elif defined(__AVR_ATmega48__) | defined(__AVR_ATmega168__) | defined(__AVR_ATmega644__) | defined(__AVR_ATmega644P__)
	#define UCSRB UCSR0B
	#define UCSRC UCSR0C
	#define UDR UDR0
	#define UBRRH UBRR0H
	#define UBRRL UBRR0L
	#define TXEN TXEN0
	#define RXEN RXEN0
	#define UCSRA UCSR0A
	#define UDRIE UDRIE0
	#define UCSZ0 UCSZ00
	#define RXCIE RXCIE0
	#define TXCIE TXCIE0
	#define RXC RXC0
	#define UDRE UDRE0
	#define UART_UDRE_VECTOR USART0_UDRE_vect
	#define UART_RECV_VECTOR USART0_RX_vect
#elif defined(__AVR_ATmega162__)
	#define UCSRB UCSR0B
	#define UCSRC UCSR0C
	#define UDR UDR0
	#define UBRRH UBRR0H
	#define UBRRL UBRR0L
	#define TXEN TXEN0
	#define RXEN RXEN0
	#define UCSRA UCSR0A
	#define UDRIE UDRIE0
	#define UCSZ0 UCSZ00
	#define UCSZ1 UCSZ01
	#define RXCIE RXCIE0
	#define TXCIE TXCIE0
	#define RXC RXC0
	#define UDRE UDRE0
	#define UART_UDRE_VECTOR USART0_UDRE_vect
	#define UART_RECV_VECTOR USART0_RXC_vect
	#define URSEL URSEL0
#elif defined(__AVR_ATmega8__) | defined(__AVR_ATmega32__) 
	#define UART_UDRE_VECTOR USART_UDRE_vect
	#define UART_RECV_VECTOR USART_RXC_vect
	#define UART_TXC_VECTOR USART_TXC_vect
#elif defined(__AVR_ATtiny2313__) | defined(__AVR_ATtiny2313A__)
	#define UART_UDRE_VECTOR USART_UDRE_vect
	#define UART_RECV_VECTOR USART_RX_vect 
#else
	#define UART_UDRE_VECTOR USART_UDRE_vect
	#define UART_RECV_VECTOR USART_RXC_vect

	#warning Your AVR is not officially supported by the uart-lib, it might not work.
#endif


#ifdef UART_BAUD_RATE
	#define BAUD UART_BAUD_RATE
#endif

#include <util/setbaud.h>


#ifdef USE_9_BITS
	#define UART_DATA_T uint16_t
#else
	#define UART_DATA_T uint8_t
#endif


#ifdef UART_INTERRUPT
volatile static UART_DATA_T rxbuf[UART_RXBUFSIZE];
volatile static UART_DATA_T txbuf[UART_TXBUFSIZE];
volatile static UART_DATA_T *volatile rxhead, *volatile rxtail;
volatile static UART_DATA_T *volatile txhead, *volatile txtail;

#ifdef USE_RS485
ISR(UART_TXC_VECTOR)
{
	RS485_DE_PORT &= ~(1 << RS485_DE_PIN); // disable RS485 DE
	UCSRB &= ~(1<<TXCIE); // disable tx complete interrupt
}
#endif

ISR(UART_UDRE_VECTOR)
{
	if (txhead == txtail)
	{
		UCSRB &= ~(1 << UDRIE);// disable data register empty IRQ
#ifdef USE_RS485
        UCSRA = (UCSRA & 0x03) | (1<<TXC); // reset tx complete flag
		UCSRB |= (1<<TXCIE); // enable tx complete interrupt
#endif
	} else
	{
		UART_DATA_T d = *txtail;
#ifdef USE_9_BITS
		if (d & 0x100) {
			UCSRB |= (1 << TXB8);  // Set TXB8 to 1
		} else {
			UCSRB &= ~(1 << TXB8); // Clear TXB8 (set to 0)
		}
#endif
		
		UDR = d & 0xff;
		if (++txtail == (txbuf + UART_TXBUFSIZE))
			txtail = txbuf;
	}
}

ISR(UART_RECV_VECTOR)
{
	int diff;

	/* buffer full? */
	diff = rxhead - rxtail;
	if ( diff < 0 )
		diff += UART_RXBUFSIZE;
	if (diff < UART_RXBUFSIZE - 1)
	{
	    // buffer NOT full
		UART_DATA_T val = 0;

#ifdef USE_9_BITS
		if ((UCSRB & (1 << RXB8)))  // Check if 9th bit is set
			val = 0x100;
#endif
		val |= UDR;	
		
		*rxhead = val;
		if (++rxhead == (rxbuf + UART_RXBUFSIZE))
			rxhead = rxbuf;
	} else
	{
		UDR;				//reads the buffer to clear the interrupt condition
	}
}

#endif // UART_INTERRUPT


void uart_init()
{
#ifdef USE_RS485
	RS485_DE_DDR |= (1 << RS485_DE_PIN);   // Set DE as output
    RS485_DE_PORT &= ~(1 << RS485_DE_PIN); // Start in receive mode
#endif
	PORTD |= 0x01;				//Pullup an RXD an

	UCSRB = _BV(TXEN) | _BV(RXEN); // UART RX und TX einschalten

	UBRRH = UBRRH_VALUE;
	UBRRL = UBRRL_VALUE;

#if USE_2X
	UCSRA = _BV(U2X);
#else
	UCSRA = 0;
#endif

#ifdef URSEL
	UCSRC = _BV(URSEL) | _BV(UCSZ1) | _BV(UCSZ0);	// Asynchron 8N1
#else
	UCSRC = _BV(UCSZ1) | _BV(UCSZ0);
#endif

#ifdef USE_9_BITS
	UCSRB |= (1 << UCSZ2);
#endif

#ifdef UART_INTERRUPT
	// init buffers
	rxhead = rxtail = rxbuf;
	txhead = txtail = txbuf;

	// activate rx IRQ
	UCSRB |= _BV(RXCIE);
#endif // UART_INTERRUPT
}

#ifdef UART_INTERRUPT
void uart_putc(UART_DATA_T c)
{
#ifdef USE_RS485
	RS485_DE_PORT |= (1 << RS485_DE_PIN); // enable DE
#endif
	volatile int diff;

	/* buffer full? */
	do
	{
		diff = txhead - txtail;
		if (diff < 0)
			diff += UART_TXBUFSIZE;
	} while (diff >= UART_TXBUFSIZE - 1);

	cli();
	*txhead = c;
	if (++txhead == (txbuf + UART_TXBUFSIZE))
		txhead = txbuf;

	UCSRB |= _BV(UDRIE);		/* enable data register empty IRQ */
	sei();
}
#else  // WITHOUT INTERRUPT
void uart_putc(UART_DATA_T c)
{
	while (!(UCSRA & _BV(UDRE)));

#ifdef USE_9_BITS
	if (c & 0x100) {
        UCSRB |= (1 << TXB8);  // Set TXB8 to 1
    } else {
        UCSRB &= ~(1 << TXB8); // Clear TXB8 (set to 0)
    }
#endif

	UDR = c & 0xff;
}
#endif // UART_INTERRUPT


void uart_putstr(char *str)
{
	while (*str)
	{
		uart_putc(*str++);
	}
}

void uart_putstr_P(PGM_P str)
{
	char tmp;
	while((tmp = pgm_read_byte(str)))
	{
		uart_putc(tmp);
		str++;
	}
}

#ifdef UART_INTERRUPT
UART_DATA_T uart_getc()
{
	UART_DATA_T val;

	while (rxhead==rxtail);

	val = *rxtail;
	if (++rxtail == (rxbuf + UART_RXBUFSIZE))
		rxtail = rxbuf;

	return val;

}
#else  // WITHOUT INTERRUPT
UART_DATA_T uart_getc()
{
	UART_DATA_T val = 0;
	while (!(UCSRA & (1<<RXC)));

#ifdef USE_9_BITS
	if ((UCSRB & (1 << RXB8)))  // Check if 9th bit is set
		val = 0x100;
#endif

	val |= UDR;
	
		
	return val;
}		
#endif // UART_INTERRUPT

// returns 1 on success
#ifdef UART_INTERRUPT
char uart_getc_nb(UART_DATA_T *c)
{
	if (rxhead == rxtail)
		return 0;

	*c = *rxtail;
	if (++rxtail == (rxbuf + UART_RXBUFSIZE))
		rxtail = rxbuf;

	return 1;
}
#else  // WITHOUT INTERRUPT
char uart_getc_nb(UART_DATA_T *c)
{
	if (UCSRA & _BV(RXC))		// Zeichen verfuegbar
	{
		UART_DATA_T val = UDR;
		
#ifdef USE_9_BITS
		if ((UCSRB & (1 << RXB8)))  // Check if 9th bit is set
			val |= 0x100;
#endif

		*c = val;
		return 1;
	}

	return 0;
}
#endif // UART_INTERRUPT


#ifdef UART_HEXDUMP
//hexdump utility
void uart_hexdump(unsigned char *buf, int len)
{
	unsigned char x=0;
	char sbuf[3];

	while (len--)
	{
		itoa(*buf++, sbuf, 16);
		if (sbuf[1] == 0) uart_putc(' ');
		uart_putstr(sbuf);
		uart_putc(' ');
		if (++x == 16)
		{
			uart_putstr_P(PSTR("\r\n"));
			x = 0;
		}
	}
}
#endif

#ifdef UART_GETLINE
//get one Cariage return terminated line
//echo charakters back on Uart
//returns buffer with zero terminated line on success, 0 pointer otherwise
char *uart_getline()
{
	static char buffer[UART_LINE_BUFFER_SIZE];
	static char *pos = buffer;
	char tmp;
	while ((tmp = uart_getc()))
	{
		if(tmp == '\n')
		{
			*pos = 0;	//terminate line
			pos = buffer;   //reset pointer
			return buffer;  //and return the buffer
		}
		if ((tmp != '\r') && (pos < buffer+UART_LINE_BUFFER_SIZE-1)) //buffer full?
		{
			*pos++ = tmp;		//no: write character to buffer
			//uart_putc (tmp);
		}
	}
	return 0;
}
#endif


#ifdef UART_INTERRUPT
char uart_rxbuffer_notempty(void)
{
	if (rxhead==rxtail)
		return 0;
	else
		return 1;
}
#endif

#ifdef UART_INTERRUPT
#if UART_RXBUFSIZE < 255
uint8_t uart_getrxqueuelength(void) 
#else
uint16_t uart_getrxqueuelength(void) 
#endif
{
	return (rxhead - rxtail);
}
#endif

