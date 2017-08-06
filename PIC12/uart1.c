#include "uart1.h"							//we use uart1 software uart

//hardware configuration
//end hardware configuration

//global defines

//global variables
volatile uint8_t _uart_busy=0;				//uart busy flag
volatile uint16_t _uart_txbuf;				//uart transmission buffer
volatile char *_uart_txstr;					//transmission string / pointer
volatile uint16_t _uart_mask;				//current bit being transmitted
volatile uint16_t _uart_pr;					//uart period, msb only
uint16_t _uart_pr0;

//global isr
//to be included in user code
//void interrupt isr(void) {
//	if (TMR1IF) uart1_isr();
//}
	
//uart1 isr
void uart1_isr(void) {
	TMR1IF = 0;								//clear the flag
	_uart_mask = _uart_mask << 1;		//shift to the next bit
	if (_uart_mask != (1<<10)) {		//done with the stop bit / current char
		TMR1 += -_uart_pr0;				//load the lsb
		//_uart_pr = _uart_pr0 &~0x00ff;	//clear the lsb
		if (_uart_txbuf & _uart_mask) IO_SET(UART1_PORT, UART1_TX);
		else IO_CLR(UART1_PORT, UART1_TX);
	} else {							//transmitt the next bit
		//_uart_txbuf has been transmitted
		//check to see if we are at end of the string
		if (*_uart_txstr!=0) {			//more chars to transmit
			uart1_put(*_uart_txstr++);
		} else {						//end of the string has been reached
			TMR1IE = 0;					//turn off the isr
			_uart_busy=0;				//uart is no longer busy
		}	
	}
}

	
//initialize the uart/tmr1
void uart1_init(uint32_t bps) {
	//TX idles high
	IO_SET(UART1_PORT, UART1_TX); 
	IO_OUT(UART1_DDR, UART1_TX);
	
	_uart_busy=0;							//0->uart is not busy
	
	//configure timer
	_uart_pr0 = F_CPU / bps;				//calculat the period
	TMR1ON = 0;								//0->tmr1 off, 1->tmr1 on
	TMR1CS = 0;								//0->count internal clock
	T1CKPS1=T1CKPS0 = 0;					//0b00->1:1 prescaler
	T1OSCEN = 0;							//0->low frequency oscillator off
	TMR1GE = 0;								//0->tmr1 not gated
	TMR1IF = 0;								//clear the flag
	TMR1IE = 0;								//disable the isr
	//TMR0 = -_uart_pr;						//load the lsb
	//_uart_pr &=~0xff;						//clear the lsb
	PEIE = 1;								//1->peripheral interrupt enabled	
	TMR1ON = 1 ;							//1->tmr1 enabled
}

//send a uart char
//8 data bit, 1 start bit, 1 stop bit, lsb first
void uart1_put(char ch) {
	//while (_uart_busy) continue;			//wait for uart to free up
	_uart_busy=1;							//uart is now busy
	_uart_txbuf = 	(0<<0) |				//start bit = 0
					(ch << 1) | 			//8 data bits, lsb first
					(1<<9);					//1 stop bit = 1
	_uart_mask = (1<<0);					//start transmission, lsb first
	
	//configure timer
	TMR1 = -_uart_pr0;						//lowest the lsb
	TMR1IF = 0;								//clera the flag
	TMR1IE = 1;								//enable isr
	//_uart_pr =_uart_pr0 &~0x00ff;			//clear lsb
	//set / clear the tx pin
	if (_uart_txbuf & _uart_mask) IO_SET(UART1_PORT, UART1_TX);
	else IO_CLR(UART1_PORT, UART1_TX);
	//the rest of the transmission done in the isr
}

//send a string - non blocking
void uart1_puts(char *str) {
	//while (*str) uart1_put(*str++);		//block transmission
	_uart_txstr=str;						//point to the string
	uart1_put(*_uart_txstr++);				//start the transmission
}

//return 1 if uart is busy
char uart1_busy(void) {
	return		(_uart_busy!=0);
}
