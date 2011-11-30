#ifndef WATCHDOG_H
#define WATCHDOG_H

/* FreeRTOS stuff */
#include "FreeRTOS.h"
#include "semphr.h"

/* Makebed stuff */
#include "MakebedConfig.h"

/* LPC Register Definitions */
#include "LPC17xx.h"



/* Clock-frequency of watch dog timer */
#define WATCHDOG_TIMER_HZ ((configCPU_CLOCK_HZ / 4) / 4)

/* Number of cycles during which the watchdog must be reset */
#define WATCHDOG_TIMER_VALUE ((WATCHDOG_FEED_INTERVAL_MS * WATCHDOG_TIMER_HZ) / 1000)



/**
 * Initialise, enable and start the watchdog timer.
 */
void watchdog_init(void);

/**
 * Feed the watchdog.
 */
void watchdog_feed(void);




#endif
