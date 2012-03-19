#include <avr/interrupt.h>  
#include <avr/io.h>

#include "uart.h"



void uart_puts (const char *s)
{
    do
    {
        uart_putc (*s);
    }
    while (*s++);
}



unsigned short getAnalogVal(unsigned char port)
{
	unsigned short retval;
	unsigned char vl, vh;


	ADMUX = port;

	ADCSRA |= (1 << ADSC);
	while((ADCSRA & (1 << ADSC)) != 0);

	vl = ADCL;
	vh = ADCH;

	ADCSRA |= (1 << ADSC);
	while((ADCSRA & (1 << ADSC)) != 0);

	vl = ADCL;
	vh = ADCH; //measure twice for stable value

	

	retval = ((vh & 3) << 8) | vl;

	return retval;
}



int main(void)
{
	uart_init();

	sei();
    
	ADCSRA = (1 << ADEN);
	

	for(;;)
	{
		uart_getc_wait();

		unsigned short data[6];
		data[0] = getAnalogVal(0);
		data[1] = getAnalogVal(1);
		data[2] = getAnalogVal(2);
		data[3] = getAnalogVal(3);
		data[4] = getAnalogVal(4);
		data[5] = getAnalogVal(5);

		uart_send_pkg(UART_PKG_ID_BATCH, data, sizeof(data));
		
	}
	
	return 0;
}
