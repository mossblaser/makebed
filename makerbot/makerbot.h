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
#include "queue.h"

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

/* Number of items to put into the command queue of the printer. */
#define MAKERBOT_COMMAND_QUEUE_LENGTH ((MAKERBOT_COMMAND_QUEUE_SIZE) / sizeof(makerbot_command_t))


/**
 * Types of instructions which the makerbot can process.
 */
typedef enum {
	MAKERBOT_NOP,
	MAKERBOT_SET_ORIGIN,
	MAKERBOT_MOVE_TO,
	MAKERBOT_SET_TEMPERATURE,
	MAKERBOT_WAIT_HEATERS,
	MAKERBOT_SLEEP,
	MAKERBOT_SET_EXTRUDER,
	MAKERBOT_SET_PLATFORM,
	MAKERBOT_SET_POWER,
	MAKERBOT_SET_AXES_ENABLED,
} makerbot_instr_t;


/**
 * Commands queued up by the printer
 */
typedef struct {
	// The instruction
	makerbot_instr_t instr;
	
	// Arguments
	union {
		// NOP
		struct { } nop;
		
		// Set Origin
		struct {
			int offset_steps[MAKERBOT_NUM_AXES];
		} set_origin;
		
		// Move To
		struct {
			stepper_dir_t directions  [MAKERBOT_NUM_AXES];
			int           num_steps   [MAKERBOT_NUM_AXES];
			int           step_periods[MAKERBOT_NUM_AXES];
		} move_to;
		
		// Set Temperature
		struct {
			int    heater_num;
			double temperature;
		} set_temperature;
		
		// Wait for Heaters
		struct { } wait_heaters;
		
		// Sleep
		struct {
			int duration;
		} sleep;
		
		// Set Extruder
		struct {
			bool enabled;
		} set_extruder;
		
		// Set Platform
		struct {
			bool enabled;
		} set_platform;
		
		// Set Power
		struct {
			bool enabled;
		} set_power;
		
		// Axes Enabled
		struct {
			bool enabled;
		} set_axes_enabled;
	} arg;
} makerbot_command_t;


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
	
	// The current temperature
	double temperature;
	
	// Heater has reached the target temperature
	bool reached;
} makerbot_heater_t;


/**
 * State of the makerbot.
 */
typedef struct makerbot {
	// Commands queued up for execution
	xQueueHandle     command_queue;
	xSemaphoreHandle command_queue_lock;
	
	// Buffer Underrun Counter
	int buffer_underruns;
	
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
	
	// Pins the PSU & PSU status signal are on
	gpio_t *psu;
	gpio_t *psu_ok;
} makerbot_t;

/**
 * Initialise the makerbot: sets up all IO, heaters, steppers & relevent
 * calibration data. PSU remains off. Call after initialising subsystems for
 * watchdog, gpio, analog_in and steppers.
 */
void makerbot_init(void);

/**
 * A task which processes instructions off the queue.
 */
void makerbot_main_task(void *pvParameters);

/**
 * A task which maintains heater temperature and feeds the watchdog.
 */
void makerbot_pid_task(void *pvParameters);

/**
 * Run the PID controller for the heaters and set heaters as appropriate. For
 * internal use only.
 */
void _makerbot_pid(double dt);

/**
 * Clear the queue of instructions and turn off all peripherals (leaves head in
 * undefined position).
 */
void makerbot_reset(void);

/**
 * Reset the buffer underrun counter.
 */
void makerbot_reset_underruns(void);

/**
 * Get the number of buffer underruns.
 */
int makerbot_buffer_underruns(void);

/**
 * Get the number of commands in the buffer.
 */
size_t makerbot_queue_length(void);

/**
 * Append an instruction to the queue to set the current position (mm) - offset
 * as the origin.
 */
void makerbot_set_origin(double offset_mm[MAKERBOT_NUM_AXES]);

/**
 * Append an instruction to the queue to move to a given position at the speed
 * specified.
 */
void makerbot_move_to(double pos_mm[MAKERBOT_NUM_AXES], double speed_mm_s);

/**
 * Get the position of the makerbot after all queued commands have been executed
 */
double makerbot_get_position(int axis);

/**
 * Append an instruction to the queue to set the temperature of one of the
 * heaters (*c)
 */
void makerbot_set_temperature(int heater_num, double temperature);
 
/**
 * Get the current temperature of one of the heaters (*c)
 */
double makerbot_read_temperature(int heater_num);
 
/**
 * Get the last-read temperature of one of the heaters (*c)
 */
double makerbot_get_temperature(int heater_num);
 
/**
 * Get the current target temperature of one of the heaters (*c)
 */
double makerbot_get_set_point(int heater_num);

/**
 * Are the heaters at the right temperature (for internal use)
 */
bool _makerbot_temperature_reached(int heater_num);

/**
 * Add an instruction to block until all heaters are up to temperature.
 */
void makerbot_wait_heaters(void);

/**
 * Add an instruction to sleep for a certain amount of time (ms).
 */
void makerbot_sleep(int duration);

/**
 * Add an instruction to turn the extruder on/off.
 */
void makerbot_set_extruder(bool enabled);

/**
 * Add an instruction to turn the belt on/off.
 */
void makerbot_set_platform(bool enabled);

/**
 * Add an instruction to the queue to turn the power on/off (leaving head in
 * unknown position).
 */
void makerbot_set_power(bool enabled);

/**
 * Get the state of the PSU (internal use only)
 */
bool makerbot_get_power(void);

/**
 * Adds an instruction to the queue to enable/disable axes
 */
void makerbot_set_axes_enabled(bool enabled);

#endif // def MAKERBOT_H
