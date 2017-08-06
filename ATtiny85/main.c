//#include "config.h"						//configuration words
#include "gpio.h"                           //we use gpio functions
#include "delay.h"                          //we use software delays
#include "uart0.h"							//we use software uart0
#include "uart1.h"							//we use software uart1

//hardware configuration
#define LED_PORT		PORTB
#define LED_DDR			DDRB
#define LED				(1<<2)
//end hardware configuration

//global defines

//global variables

//global isr

int main(void) {

	mcu_init();							    //initialize the mcu
	//uart0_init(UARTBPS_1200);				//initialize the uart0
	uart1_init(UARTBPS_1200);				//initialize the uart1
	IO_OUT(LED_DDR, LED);
	ei();
	while (1) {
		//if (!uart0_busy()) uart0_puts((char *)"Software UART on TMR0!\n\r");
		if (!uart1_busy()) uart1_puts((char *)"Software UART on TMR1!\n\r");
		//IO_FLP(LED_PORT, LED);
	}
}

