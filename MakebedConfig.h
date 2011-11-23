#ifndef MAKEBED_CONFIG_H
#define MAKEBED_CONFIG_H

#include "FreeRTOSConfig.h"


/*******************************************************************************
 * Stepper Motor Control Configuration
 ******************************************************************************/

/* How many steppers will be managed? */
#define STEPPER_NUM_MOTORS 3

/* Which system timer should be used for stepper control (0-3) */
#define STEPPER_TIMER_NO 1

/* The frequency at which the timer increments (Hz) */
#define STEPPER_TIMER_HZ 1000

/* The hold time in ns of the direction/enable signals of a stepper controller */
#define STEPPER_HOLD_NS 200



/* Calculated values, do not change! */

/* Prescaler for the timer counter (Do not change) */
#define STEPPER_PRESCALER ((configCPU_CLOCK_HZ \
                            / ((unsigned long) STEPPER_TIMER_HZ)) - 1)

/* The period of the timer tick in ns (Do not change) */
#define STEPPER_TIMER_PERIOD_NS (1000000000ul / STEPPER_TIMER_HZ)

/* Hold time in timer ticks for setting direction/enabling (Do not change) */
/* STEPPER_HOLD_TICKS = ceil(STEPPER_HOLD_NS / STEPPER_TIMER_PERIOD_NS) */
#define STEPPER_HOLD_TICKS ((((unsigned long)STEPPER_HOLD_NS) \
                             + STEPPER_TIMER_PERIOD_NS - 1ul) \
                            / STEPPER_TIMER_PERIOD_NS)


#endif
