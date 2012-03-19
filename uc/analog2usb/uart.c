#include <avr/io.h>
#include <avr/interrupt.h>
#include "uart.h"
#include "fifo.h" // erklärt im Artikel "FIFO mit avr-gcc"

#define BAUDRATE 9600

// FIFO-Objekte und Puffer für die Ein- und Ausgabe 

#define BUFSIZE_IN  0x40
uint8_t inbuf[BUFSIZE_IN];
fifo_t infifo;

#define BUFSIZE_OUT 0x40
uint8_t outbuf[BUFSIZE_OUT];
fifo_t outfifo;


// Empfangene Zeichen werden in die Eingabgs-FIFO gespeichert und warten dort 
SIGNAL (SIG_UART_RECV)
{
    _inline_fifo_put (&infifo, UDR);
}

// Ein Zeichen aus der Ausgabe-FIFO lesen und ausgeben 
// Ist das Zeichen fertig ausgegeben, wird ein neuer SIG_UART_DATA-IRQ getriggert 
// Ist die FIFO leer, deaktiviert die ISR ihren eigenen IRQ. 
SIGNAL (SIG_UART_DATA)
{
    if (outfifo.count > 0)
       UDR = _inline_fifo_get (&outfifo);
    else
        UCSRB &= ~(1 << UDRIE);
}

void uart_send_pkg(unsigned char id, unsigned char *data, unsigned char datalen)
{
	unsigned char chksum = 0;
	unsigned char i;
	
	uart_putc('S');
	uart_putc('N');
	uart_putc('P');
	
	uart_putc(id); chksum += id;
	uart_putc(datalen); chksum += datalen;
	
	for(i=0; i<datalen; ++i)
	{
		uart_putc(data[i]);
		chksum += data[i];
	}
	
	uart_putc(chksum);
}
	

void uart_init()
{
    uint8_t sreg = SREG;
    uint16_t ubrr = (uint16_t) ((uint32_t) F_CPU/(16UL*BAUDRATE) - 1);

    UBRRH = (uint8_t) (ubrr>>8);
    UBRRL = (uint8_t) (ubrr);

    // Interrupts kurz deaktivieren 
    cli();

    // UART Receiver und Transmitter anschalten, Receive-Interrupt aktivieren 
    // Data mode 8N1, asynchron 
    UCSRB = (1 << RXEN) | (1 << TXEN) | (1 << RXCIE);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

    // Flush Receive-Buffer (entfernen evtl. vorhandener ungültiger Werte) 
    do
    {
        // UDR auslesen (Wert wird nicht verwendet) 
        UDR;
    }
    while (UCSRA & (1 << RXC));

    // Rücksetzen von Receive und Transmit Complete-Flags 
    UCSRA = (1 << RXC) | (1 << TXC);

    // Global Interrupt-Flag wieder herstellen 
    SREG = sreg;

    // FIFOs für Ein- und Ausgabe initialisieren 
    fifo_init (&infifo,   inbuf, BUFSIZE_IN);
    fifo_init (&outfifo, outbuf, BUFSIZE_OUT);
}


int uart_putc (const uint8_t c)
{
    int ret = fifo_put (&outfifo, c);
	
    UCSRB |= (1 << UDRIE);
	 
    return ret;
}

int uart_getc_nowait ()
{
    return fifo_get_nowait (&infifo);
}

uint8_t uart_getc_wait ()
{
    return fifo_get_wait (&infifo);
}

