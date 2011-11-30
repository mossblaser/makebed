/**
 * Parameters, constands and pin allocations for the makebed software.
 */

#ifndef MAKEBED_CONFIG_H
#define MAKEBED_CONFIG_H


/*******************************************************************************
 * Watchdog Falisafe Configuration
 ******************************************************************************/

/* Watchdog feed interval */
#define WATCHDOG_FEED_INTERVAL_MS 2000



/*******************************************************************************
 * Power Control Configuration
 ******************************************************************************/

/* Pin allocations. */

/* Power Enable pin (active high) for the PSU. */
#define PIN_POWER_EN (&gpio_mbed_p24)

/* Signal (active high) from PSU indicating normal power flow. */
#define PIN_POWER_OK (&gpio_mbed_p5)



/*******************************************************************************
 * Stepper Motor Control Configuration
 ******************************************************************************/

/* !Enable pins for stepper motors */
#define PIN_STEPPER_0_NEN (&gpio_mbed_p10)
#define PIN_STEPPER_1_NEN (&gpio_mbed_p15)
#define PIN_STEPPER_2_NEN (&gpio_mbed_p23)

/* Direction pins for stepper motors */
#define PIN_STEPPER_0_DIR (&gpio_mbed_p9)
#define PIN_STEPPER_1_DIR (&gpio_mbed_p14)
#define PIN_STEPPER_2_DIR (&gpio_mbed_p22)

/* Enable pins for stepper motors */
#define PIN_STEPPER_0_STEP (&gpio_mbed_p8)
#define PIN_STEPPER_1_STEP (&gpio_mbed_p13)
#define PIN_STEPPER_2_STEP (&gpio_mbed_p21)


/* Stepper controler parameters. */

/* How many steppers will be managed? */
#define STEPPER_NUM_MOTORS 3

/* Which system timer should be used for stepper control (0-3) */
#define STEPPER_TIMER_NO 1

/* The frequency at which the timer increments (Hz) */
#define STEPPER_TIMER_HZ 1000

/* The hold time in ns of the direction/enable signals of a stepper controller */
#define STEPPER_HOLD_NS 200


#endif
