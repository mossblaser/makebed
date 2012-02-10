#ifndef GCODE_H
#define GCODE_H

/* Standard includes. */
#include "stdio.h"
#include "stdbool.h"
#include "ctype.h"
#include "math.h"

/* XXX: Use custom float parsing as libc broken. */
#include "strtod.h"

/* FreeRTOS stuff */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/* Makebed stuff */
#include "MakebedConfig.h"

/* HW Libraries */
#include "makerbot.h"

/* Size of the buffer for numerical values. */
#define GCODE_VALUE_BUF_SIZE 16

/* Number of inches per mm */
#define GCODE_IN_PER_MM 0.0393700787


/**
 * Errors which can occur during processing.
 */
typedef enum {
	GCODE_ERR_NONE, // No error has occurred
	GCODE_ERR_INVALID_REG, // An unrecognised register appeared
	GCODE_ERR_INVALID_VALUE, // An un-parseable value for a reg was used
	GCODE_ERR_COMMAND_MISSING, // G or M weren't specified
	GCODE_ERR_UNKNOWN_GCODE, // The gcode used wasn't recognised
	GCODE_ERR_UNKNOWN_MCODE, // The gcode used wasn't recognised
	GCODE_ERR_RELATIVE, // We don't support relative positioning
	GCODE_ERR_EXTRUDER_SPEED, // We don't support non-255 extruder speeds
	GCODE_ERR_EXTRUDER_REVERSE, // Don't support reverse driving the extruder
} gcode_error_t;


/**
 * Registers in the gcode interpreter
 */
typedef struct {
	// Is the register integral or floating-point?
	bool is_integer;
	
	// Value held by the register
	union {
		int    i;
		double d;
	} value;
} gcode_reg_t;


/**
 * Gcode parser object.
 */
typedef struct {
	// Queue of unprocessed chars of input gcode
	xQueueHandle queue;
	
	// The last error that occurred
	gcode_error_t error;
	
	// The register bank
	gcode_reg_t registers['Z'-'A' + 1];
	
	// The conversion from the current unit to mm
	double unit_per_mm;
} gcode_t;


/**
 * Intialise the gcode interpreter.
 */
void gcode_init(void);


/**
 * A task which continually interprets gcode added to the queue. This contains
 * the mainloop of the interpreter.
 */
void gcode_task(void *pvParameters);


/**
 * Push a string onto the gcode interpreter queue
 */
void gcode_interpret(char *code, size_t len);


/* Internal Use Only **********************************************************/

/**
 * Upon an error occuring, scan to the end of line
 */
void _gcode_error(gcode_error_t error);

/**
 * Ignore until the new-line
 */
void _gcode_ignore_line(void);

/**
 * Ignore until and including a paricular is read
 */
void _gcode_ignore_until(char s);

/**
 * Read/parse the current int.
 */
bool _gcode_read_int(int *value);

/**
 * Read/parse the current double.
 */
bool _gcode_read_double(double *value);

/**
 * Process the registers in the system (i.e. interpret the last command read)
 */
void _gcode_step(void);

/**
 * Process the registers in the system when G is set
 */
void _gcode_step_m(void);

/**
 * Process the registers in the system when M is set
 */
void _gcode_step_g(void);


#endif // def GCODE_H
