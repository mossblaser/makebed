#include "watchdog.h"


void
watchdog_init(void)
{
	// Set the timeout for the watchdog
	LPC_WDT->WDTC = WATCHDOG_TIMER_VALUE;
	
	// Enable the watchdog timer and make it cause a reset when it underflows
	LPC_WDT->WDMOD |= 0x3;
	
	watchdog_feed();
}


void
watchdog_feed(void)
{
	portENTER_CRITICAL();
	
	// Execute the feed sequence
	LPC_WDT->WDFEED = 0xAA;
	LPC_WDT->WDFEED = 0x55;
	
	portEXIT_CRITICAL();
}
