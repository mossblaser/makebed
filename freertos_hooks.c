/**
 * Makebed -- A Makerbot control firmware for the mbed
 * (C) 2011 Jonathan Heathcote, University of Manchester
 */

#include "freertos_hooks.h"


/* The time between cycles of the 'check' functionality (defined within the
tick hook. */
#define CHECK_DELAY ( ( portTickType ) 5000 / portTICK_RATE_MS )

void
vApplicationTickHook(void)
{
	static unsigned long ulTicksSinceLastDisplay = 0;
	
	// Have enough ticks passed to make it	time to perform our health status
	// check again?
	
	ulTicksSinceLastDisplay++;
	if( ulTicksSinceLastDisplay >= CHECK_DELAY ) {
		ulTicksSinceLastDisplay = 0;
		
		// TODO: Do something here...
	}
}


void
vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName)
{
	( void ) pxTask;
	( void ) pcTaskName;
	
	// TODO: Do something more informative than infiniloop
	for( ;; );
}


void
vConfigureTimerForRunTimeStats(void)
{
	const unsigned long TCR_COUNT_RESET = 2;
	const unsigned long CTCR_CTM_TIMER = 0x00;
	const unsigned long TCR_COUNT_ENABLE = 0x01;
	
	/* Power up and feed the timer. */
	LPC_SC->PCONP |= 0x02UL;
	LPC_SC->PCLKSEL0 = (LPC_SC->PCLKSEL0 & (~(0x3<<2))) | (0x01 << 2);
	
	/* Reset Timer 0 */
	LPC_TIM0->TCR = TCR_COUNT_RESET;
	
	/* Just count up. */
	LPC_TIM0->CTCR = CTCR_CTM_TIMER;
	
	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	LPC_TIM0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;
	
	/* Start the counter. */
	LPC_TIM0->TCR = TCR_COUNT_ENABLE;
}

