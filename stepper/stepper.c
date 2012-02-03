#include "stepper.h"

/* Utility macros */
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))


// Array of stepper motors controled by the system
static stepper_t steppers[STEPPER_NUM_MOTORS];

// A mutex used to allow tasks to wait on the stepper motor control to become
// idle
static xSemaphoreHandle stepper_idle_mutex = NULL;

// A counter of how many steps have been missed due to timing faliures
static int stepper_missed_steps = 0;


void
stepper_init(void)
{
	vSemaphoreCreateBinary(stepper_idle_mutex);
	xSemaphoreTake(stepper_idle_mutex, portMAX_DELAY);
	stepper_missed_steps = 0;
	
	// Power-on the timer
	LPC_SC->PCONP |= STEPPER_TIMER_PCON;
	
	// Set the clock divider into the timer to use the CPU clock speed
	STEPPER_TIMER_PCLKSEL = (STEPPER_TIMER_PCLKSEL & ~STEPPER_TIMER_PCLKSEL_MASK)
	                        | STEPPER_TIMER_PCLKSEL_VAL;
	
	// Reset & disable counter/timer
	STEPPER_TIMER_REG->TCR = 2;
	
	// Enable interrupts from the counter timer
	NVIC_EnableIRQ(STEPPER_TIMER_IRQn); // Enable interupt
	NVIC_SetPriority(STEPPER_TIMER_IRQn, configSTEPPER_INTERRUPT_PRIORITY);
	
	// Set prescaler
	STEPPER_TIMER_REG->PR = STEPPER_PRESCALER;
	
	// Reset and interrupt on MR0 match
	STEPPER_TIMER_REG->MCR = 3;
}


void
stepper_init_motor(size_t s_num, gpio_t *nen, gpio_t *dir, gpio_t *step)
{
	// TODO: Handle error condition more gracefully
	while (s_num >= STEPPER_NUM_MOTORS)
		;
	
	// Set up variables
	steppers[s_num].steps_remaining = 0;
	steppers[s_num].step_period = 0;
	steppers[s_num].next_toggle = 0;
	
	// Set IO pins
	steppers[s_num].pin_nen  = nen;
	steppers[s_num].pin_dir  = dir;
	steppers[s_num].pin_step = step;
	
	// Set up the pins
	gpio_set_mode(steppers[s_num].pin_nen,  GPIO_OUTPUT);
	gpio_set_mode(steppers[s_num].pin_dir,  GPIO_OUTPUT);
	gpio_set_mode(steppers[s_num].pin_step, GPIO_OUTPUT);
	
	// Set default pin states (disabled)
	gpio_write(steppers[s_num].pin_nen,  GPIO_HIGH);
	gpio_write(steppers[s_num].pin_dir,  STEPPER_FORWARD);
	gpio_write(steppers[s_num].pin_step, GPIO_LOW);
}


int
stepper_get_missed_steps(void)
{
	return stepper_missed_steps;
}


void
stepper_set_enabled(size_t s_num, bool enabled)
{
	// TODO: Handle error condition more gracefully
	while (s_num >= STEPPER_NUM_MOTORS)
		;
	
	// Stop the motor moving
	steppers[s_num].steps_remaining = 0;
	
	// Set the pins appropriately
	// XXX: May violate hold time for enable and/or step if set just after an edge
	gpio_write(steppers[s_num].pin_nen, enabled ? GPIO_LOW : GPIO_HIGH);
	gpio_write(steppers[s_num].pin_step, GPIO_LOW);
}


bool
stepper_get_enabled(size_t s_num)
{
	// TODO: Handle error condition more gracefully
	while (s_num >= STEPPER_NUM_MOTORS)
		;
	
	return !gpio_read(steppers[s_num].pin_nen);
}


void
stepper_set_action(size_t        s_num,
                   stepper_dir_t direction,
                   unsigned int  num_steps,
                   int           step_period)
{
	taskENTER_CRITICAL();
	
	// TODO: Handle error condition more gracefully
	while (s_num >= STEPPER_NUM_MOTORS)
		;
	
	// XXX: This function may violate hold times if the last step from this motor
	// was very recent. (But it won't violate hold times for the following step.)
	// Could check and then wait...?
	
	// Enable the motor
	gpio_write(steppers[s_num].pin_nen, GPIO_LOW);
	
	// Reset the step to low (i.e. ready for a new step)
	gpio_write(steppers[s_num].pin_step, GPIO_LOW);
	
	// Set the direction of the stepper
	gpio_write(steppers[s_num].pin_dir, direction);
	
	// The duration of a step
	steppers[s_num].step_period = step_period;
	
	// Set up the number of steps remaining
	steppers[s_num].steps_remaining = num_steps;
	
	// Make the first step after the hold-time expires
	steppers[s_num].next_toggle = STEPPER_TIMER_REG->TC + STEPPER_HOLD_TICKS;
	
	// Update the timer match register to the new next toggle
	int next_toggle = steppers[s_num].next_toggle;
	int s_num_i;
	for (s_num_i = 0; s_num_i < STEPPER_NUM_MOTORS; s_num_i++)
		if (steppers[s_num_i].steps_remaining > 0)
			next_toggle = MIN(steppers[s_num_i].next_toggle, next_toggle);
	STEPPER_TIMER_REG->MR0 = next_toggle;
	
	// Enable counter/timer
	STEPPER_TIMER_REG->TCR = 1;
	
	taskEXIT_CRITICAL();
}


bool
stepper_is_idle_motor(size_t s_num)
{
	// TODO: Handle error condition more gracefully
	while (s_num >= STEPPER_NUM_MOTORS)
		;
	
	return steppers[s_num].steps_remaining == 0;
}


void
stepper_wait_until_idle(void)
{
	while (xSemaphoreTake(stepper_idle_mutex, portMAX_DELAY) != pdTRUE)
		;
}


/**
 * ISR for controlling the steppers.
 *
 * If this routine does not complete before the next timer interrupt, the result
 * is undefined (and steps will be missed). It is therefore important that this
 * routine will always finish well before the timer next expires.
 */
void
STEPPER_TIMER_IRQ_HANDLER(void)
{
	// This has the effect of resetting the counter on the next increment. This is
	// required as the match register is checked after every prescaler increment
	// to decide when to reset the counter. As a result, when we change the
	// match register, this reset never happens (unless we happen to pick the same
	// value).
	STEPPER_TIMER_REG->TC = 0xFFFFFFFF;
	
	// The time at which the next toggle to occur (or INT_MAX if no toggles are
	// due)
	int next_toggle = INT_MAX;
	
	// The time since the last interrupt occurred = last timer interval
	int time_delta = STEPPER_TIMER_REG->MR0;
	
	// Keep track of the number of idle motors
	int num_idle_motors = STEPPER_NUM_MOTORS;
	static int last_num_idle_motors = STEPPER_NUM_MOTORS;
	
	// Iterate over each controlled stepper, skipping ones which aren't moving
	int s_num;
	for (s_num = 0; s_num < STEPPER_NUM_MOTORS; s_num++) {
		if (steppers[s_num].steps_remaining > 0) {
			// This motor is not idle
			num_idle_motors--;
			
			// Next toggle is decrememnted by how long since the last toggle
			steppers[s_num].next_toggle -= time_delta;
			
			// Due to toggle now
			if (steppers[s_num].next_toggle <= STEPPER_MERGE_WINDOW_TICKS) {
				// Toggle the step pin
				gpio_value_t step_state = gpio_read(steppers[s_num].pin_step);
				gpio_write(steppers[s_num].pin_step, !step_state);
				
				// If the pin is now low, we've completed a step
				if ((!step_state) == GPIO_LOW) {
					steppers[s_num].steps_remaining--;
					
					// Has this motor just become idle
					if (steppers[s_num].steps_remaining == 0) {
						num_idle_motors++;
					}
				}
				
				// Toggle next in half a step period
				steppers[s_num].next_toggle += (steppers[s_num].step_period>>1) - 1;
				
				// Have we missed a step?
				if (steppers[s_num].next_toggle <= 0) {
					stepper_missed_steps++;
					steppers[s_num].next_toggle = (steppers[s_num].step_period>>1) - 1;
				}
			}
			
			// Keep track of the soonest-occuring interrupt
			if (steppers[s_num].steps_remaining > 0)
				next_toggle = MIN(steppers[s_num].next_toggle, next_toggle);
			
		}
	}
	
	
	// Are all motors now idle?
	if (num_idle_motors == STEPPER_NUM_MOTORS || next_toggle == INT_MAX) {
		// Reset & disable counter/timer
		STEPPER_TIMER_REG->TCR = 2;
		
		// Inform the system all steppers are now idle if this has just happened
		if (num_idle_motors != last_num_idle_motors) {
			portBASE_TYPE woken_task;
			xSemaphoreGiveFromISR(stepper_idle_mutex, &woken_task);
		}
	} else {
		// Motors still have steps remaining
		// Set the time for the next interrupt
		STEPPER_TIMER_REG->MR0 = next_toggle;
	}
	
	last_num_idle_motors = num_idle_motors;
	
	// Clear the timer-counter interrupt
	STEPPER_TIMER_REG->IR = STEPPER_TIMER_REG->IR;
	
	// Clear the NVIC ready to accept a new interrupt
	NVIC_ClearPendingIRQ(STEPPER_TIMER_IRQn);
}

