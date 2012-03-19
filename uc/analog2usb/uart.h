#ifndef _UART_H_
#define _UART_H_

/*

namespace tools {
namespace tackimu {
namespace src {
namespace spi2usb {

*/

#include <avr/io.h>

extern void uart_init(void);
extern int uart_putc(const uint8_t);
extern uint8_t uart_getc_wait(void);
extern int     uart_getc_nowait(void);

static inline void uart_flush(void)
{
	while (UCSRB & (1 << UDRIE));
}


#define UART_PKG_ID_BATCH 0

void uart_send_pkg(unsigned char id, unsigned char * data, unsigned char datalen);
#endif /* _UART_H_ */
