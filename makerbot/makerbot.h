#ifndef MAKERBOT_H
#define MAKERBOT_H

/* Standard includes. */
#include "stdio.h"
#include "stdbool.h"
#include "limits.h"
#include "math.h"

/* FreeRTOS stuff */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Makebed stuff */
#include "MakebedConfig.h"

/* HW Libraries */
#include "analog_in.h"
#include "gpio.h"
#include "stepper.h"
#include "thermistor.h"
#include "pid.h"
#include "watchdog.h"


/* Number of axes the print head moves */
#define MAKERBOT_NUM_AXES 3

/* Number of heaters in the printer */
#define MAKERBOT_NUM_HEATERS 2

/* Period of the makerbot task mainloop */
#define MAKERBOT_PERIOD_MS 500
#define MAKERBOT_PERIOD (MAKERBOT_PERIOD_MS / portTICK_RATE_MS)


/**
 * State of an axis of movement in the makerbot.
 */
typedef struct makerbot_axis {
	// Number of the stepper to use (used by stepper library)
	int stepper_num;
	
	// Number of steps to take per mm
	double steps_per_mm;
	
	// Position (in steps) of the axis
	int position;
} makerbot_axis_t;


/**
 * A heater in the makerbot.
 */
typedef struct makerbot_heater {
	// Pins the heater and thermistor are attached to
	gpio_t      *heater_pin;
	gpio_t      *led_pin;
	analog_in_t *thermistor_pin;
	
	// A calibrated thermistor decoder
	thermistor_t thermistor;
	
	// Refrence voltage in thermistor potential divider
	double v_vref;
	
	// A PID Controller
	pid_state_t pid;
	
	// The target temperature
	double set_point;
	
	// Heater has reached the target temperature
	bool reached;
} makerbot_heater_t;


/**
 * State of the makerbot.
 */
typedef struct makerbot {
	// Axes of the platform/extruder
	makerbot_axis_t axes[MAKERBOT_NUM_AXES];
	
	// Heaters in the platform/extruder
	makerbot_heater_t heaters[MAKERBOT_NUM_HEATERS];
	
	xSemaphoreHandle heaters_reached;
	xSemaphoreHandle heaters_on_reach;
	
	// Pin the extruder is on
	gpio_t *extruder;
	
	// Pin the platform-belt is on
	gpio_t *platform;
	
	// Pin the PSU is on
	gpio_t *psu;
} makerbot_t;

/**
 * Initialise the makerbot: sets up all IO, heaters, steppers & relevent
 * calibration data. PSU remains off. Call after initialising subsystems for
 * watchdog, gpio, analog_in and steppers.
 */
void makerbot_init(void);

/**
 * A task which maintains heater temperature and feeds the watchdog.
 */
void makerbot_task(void *pvParameters);

/**
 * Run the PID controller for the heaters and set heaters as appropriate.
 */
void makerbot_pid(double dt);

/**
 * Turn off all peripherals (leaves head in undefined position).
 */
void makerbot_reset(void);

/**
 * Set the current position (mm) - offset as the origin.
 */
void makerbot_set_origin(double offset_mm[MAKERBOT_NUM_AXES]);

/**
 * Move to a given position at the speed specified.
 */
void makerbot_move_to(double pos_mm[MAKERBOT_NUM_AXES], double speed_mm_s);

/**
 * Get the position the print head is moving to/is at (mm)
 */
double makerbot_get_position(int axis);

/**
 * Set the temperature of one of the heaters (*c)
 */
void makerbot_set_temperature(int heater_num, double temperature);

/**
 * Get the target temperature of one of the heaters (*c)
 */
double makerbot_get_temperature_target(int heater_num);

/**
 * Get the temperature of one of the heaters (*c)
 */
double makerbot_get_temperature(int heater_num);

/**
 * Are the heaters at the right temperature
 */
bool makerbot_temperature_reached(int heater_num);

/**
 * Block until all heaters are up to temperature.
 */
void makerbot_wait_heaters(void);

/**
 * Turn the extruder on/off.
 */
void makerbot_set_extruder(bool enabled);

/**
 * Get the state of the extruder
 */
bool makerbot_get_extruder(void);

/**
 * Turn the belt on/off.
 */
void makerbot_set_platform(bool enabled);

/**
 * Get the state of the belt
 */
bool makerbot_get_platform(void);

/**
 * Turn the power on/off -- resets the makerbot (leaving head in unknown
 * position).
 */
void makerbot_set_power(bool enabled);

/**
 * Get the state of the belt
 */
bool makerbot_get_power(void);

/**
 * Enable/Disable Axes
 */
void makerbot_set_axes_enabled(bool enabled);

/**
 * Enable/Disable Axes
 */
bool makerbot_get_axes_enabled(void);

#endif // def MAKERBOT_H
