#ifndef STEPPER_H
#define STEPPER_H

/* Standard includes. */
#include "stdio.h"
#include "stdbool.h"
#include "limits.h"

/* FreeRTOS stuff */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Makebed stuff */
#include "MakebedConfig.h"

/* LPC Register Definitions */
#include "LPC17xx.h"

/* GPIO Library */
#include "gpio.h"


#if STEPPER_TIMER_NO == 0
	#define STEPPER_TIMER_REG         LPC_TIM0
	#define STEPPER_TIMER_IRQn        TIMER0_IRQn
	#define STEPPER_TIMER_IRQ_HANDLER TIMER0_IRQHandler
#elif STEPPER_TIMER_NO == 1
	#define STEPPER_TIMER_REG         LPC_TIM1
	#define STEPPER_TIMER_IRQn        TIMER1_IRQn
	#define STEPPER_TIMER_IRQ_HANDLER TIMER1_IRQHandler
#elif STEPPER_TIMER_NO == 2
	#define STEPPER_TIMER_REG         LPC_TIM2
	#define STEPPER_TIMER_IRQn        TIMER2_IRQn
	#define STEPPER_TIMER_IRQ_HANDLER TIMER2_IRQHandler
#elif STEPPER_TIMER_NO == 3
	#define STEPPER_TIMER_REG         LPC_TIM3
	#define STEPPER_TIMER_IRQn        TIMER3_IRQn
	#define STEPPER_TIMER_IRQ_HANDLER TIMER3_IRQHandler
#else
	#error "Invalid STEPPER_TIMER_NO selected"
#endif


/**
 * Stepper motor direction.
 */
typedef enum stepper_dir {
	STEPPER_BACKWARD = GPIO_LOW,
	STEPPER_FORWARD  = GPIO_HIGH
} stepper_dir_t;


/**
 * State of a stepper motor.
 */
typedef struct stepper {
	/* Logical */
	unsigned int  steps_remaining; // How many steps remain to be done
	int           step_period;     // Duration between steps
	int           next_toggle;     // Time the next toggle occurs
	
	/* Physical */
	gpio_t *pin_nen;  // Stepper controller !enable pin
	gpio_t *pin_dir;  // Stepper controller direction pin
	gpio_t *pin_step; // Stepper controller step pin
} stepper_t;


/**
 * Initialise the stepper system (run one on boot after GPIO init).
 */
void stepper_init(void);


/**
 * Initialise a stepper motor in the system.
 *
 * @param s_num The index of the motor to init
 * @param nen   The !enable GPIO pin for this motor
 * @param dir   The direction GPIO pin for this motor
 * @param step  The step GPIO pin for this motor
 */
void stepper_init_motor(size_t s_num, gpio_t *nen, gpio_t *dir, gpio_t *step);


/**
 * Get the number of steps missed. In a fully operational system, this should
 * always be 0.
 */
int stepper_get_missed_steps(void);


/**
 * Set the enable/disable state of a motor (also stops it moving)
 */
void stepper_set_enabled(size_t s_num, bool enabled);


/**
 * Get the enable/disable state of a motor
 */
bool stepper_get_enabled(size_t s_num);


/**
 * Set the stepper motor moving in a given direction, distance and speed.
 * Overwrites any previous instruction. Stepping will start immediately.
 */
void stepper_set_action(size_t        s_num,
                        stepper_dir_t direction,
                        unsigned int  num_steps,
                        int           step_period);


/**
 * Is the given stepper motor idle?
 */
bool stepper_is_idle_motor(size_t s_num);


/**
 * Block the calling task until all motors are idle. Maximum of one caller at
 * once!
 */
void stepper_wait_until_idle(void);


#endif // def STEPPER_H
