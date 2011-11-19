/**
 * Makebed -- A Makerbot control firmware for the mbed
 * (C) 2011 Jonathan Heathcote, University of Manchester
 */

#ifndef FREERTOS_HOOKS_H
#define FREERTOS_HOOKS_H

#include "FreeRTOS.h"
#include "task.h"


/**
 * Called on every hardware interrupt.
 */
void vApplicationTickHook(void);

/**
 * Called if a task overflows it's stack.
 */
void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                   signed char *pcTaskName);

/**
 * Called automatically when the scheduler is started (assuming
 * configGENERATE_RUN_TIME_STATS is set to 1) and sets up a timer for generating
 * run-time stats.
 */
void vConfigureTimerForRunTimeStats(void);

#endif
