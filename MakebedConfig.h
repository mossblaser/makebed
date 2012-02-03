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

/* Calibration steps/mm data */
#define STEPPER_0_STEPS_PER_MM 10
#define STEPPER_1_STEPS_PER_MM 10
#define STEPPER_2_STEPS_PER_MM 10


/*******************************************************************************
 * Heater/Temperature Control Configuration
 ******************************************************************************/

/* Temperature Probe Pins */
#define PIN_THERMISTOR_PLATFORM (&analog_in_mbed_p19)
#define PIN_THERMISTOR_EXTRUDER (&analog_in_mbed_p20)

/* Heater Pins */
#define PIN_HEATER_PLATFORM (&gpio_mbed_p28)
#define PIN_HEATER_EXTRUDER (&gpio_mbed_p27)

/* LED Pins */
#define PIN_LED_PLATFORM (&gpio_mbed_led3)
#define PIN_LED_EXTRUDER (&gpio_mbed_led2)

/* Thermistor Calibration Data */
#define THERMISTOR_PLATFORM_V_VREF 3.3
#define THERMISTOR_PLATFORM_R      4400.0
#define THERMISTOR_PLATFORM_R0     100000.0
#define THERMISTOR_PLATFORM_B      4072.0
#define THERMISTOR_PLATFORM_T0     (25.0 + 273.15)

#define THERMISTOR_EXTRUDER_V_VREF 3.3
#define THERMISTOR_EXTRUDER_R      4400.0
#define THERMISTOR_EXTRUDER_R0     100000.0
#define THERMISTOR_EXTRUDER_B      4072.0
#define THERMISTOR_EXTRUDER_T0     (25.0 + 273.15)

/* PID */
#define THERMISTOR_EXTRUDER_P 6.25
#define THERMISTOR_EXTRUDER_I 0.4140625
#define THERMISTOR_EXTRUDER_D 200.0

#define THERMISTOR_PLATFORM_P 6.25
#define THERMISTOR_PLATFORM_I 0.4140625
#define THERMISTOR_PLATFORM_D 200.0



/*******************************************************************************
 * Extruder
 ******************************************************************************/

/* Pin */
#define PIN_EXTRUDER (&gpio_mbed_p29)



/*******************************************************************************
 * Automated Build Platform
 ******************************************************************************/

/* Pin */
#define PIN_PLATFORM (&gpio_mbed_p30)


#endif
