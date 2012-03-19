#include "gcode.h"

#define A ('A' - 'A')
#define B ('B' - 'A')
#define C ('C' - 'A')
#define D ('D' - 'A')
#define E ('E' - 'A')
#define F ('F' - 'A')
#define G ('G' - 'A')
#define H ('H' - 'A')
#define I ('I' - 'A')
#define J ('J' - 'A')
#define K ('K' - 'A')
#define L ('L' - 'A')
#define M ('M' - 'A')
#define N ('N' - 'A')
#define O ('O' - 'A')
#define P ('P' - 'A')
#define Q ('Q' - 'A')
#define R ('R' - 'A')
#define S ('S' - 'A')
#define T ('T' - 'A')
#define U ('U' - 'A')
#define V ('V' - 'A')
#define W ('W' - 'A')
#define X ('X' - 'A')
#define Y ('Y' - 'A')
#define Z ('Z' - 'A')

static gcode_t gcode;

void
gcode_init(void)
{
	gcode.queue = xQueueCreate(GCODE_BUFFER_LENGTH, sizeof(char));
	gcode.error = GCODE_ERR_NONE;
	
	gcode.unit_per_mm = GCODE_IN_PER_MM;
	
	gcode.instructions_handled = 0;
	
	// Set up registers
	gcode.registers[A].is_integer = false;
	gcode.registers[B].is_integer = false;
	gcode.registers[C].is_integer = false;
	gcode.registers[D].is_integer = false;
	gcode.registers[E].is_integer = false;
	gcode.registers[F].is_integer = false;
	gcode.registers[G].is_integer = true;
	gcode.registers[H].is_integer = false;
	gcode.registers[I].is_integer = false;
	gcode.registers[J].is_integer = false;
	gcode.registers[K].is_integer = false;
	gcode.registers[L].is_integer = false;
	gcode.registers[M].is_integer = true;
	gcode.registers[N].is_integer = false;
	gcode.registers[O].is_integer = false;
	gcode.registers[P].is_integer = true;
	gcode.registers[Q].is_integer = false;
	gcode.registers[R].is_integer = false;
	gcode.registers[S].is_integer = false;
	gcode.registers[T].is_integer = true;
	gcode.registers[U].is_integer = false;
	gcode.registers[V].is_integer = false;
	gcode.registers[W].is_integer = false;
	gcode.registers[X].is_integer = false;
	gcode.registers[Y].is_integer = false;
	gcode.registers[Z].is_integer = false;
	
	gcode.registers[A].value.d = NAN;
	gcode.registers[B].value.d = NAN;
	gcode.registers[C].value.d = NAN;
	gcode.registers[D].value.d = NAN;
	gcode.registers[E].value.d = NAN;
	gcode.registers[F].value.d = NAN;
	gcode.registers[G].value.i = INT_MIN;
	gcode.registers[H].value.d = NAN;
	gcode.registers[I].value.d = NAN;
	gcode.registers[J].value.d = NAN;
	gcode.registers[K].value.d = NAN;
	gcode.registers[L].value.d = NAN;
	gcode.registers[M].value.i = INT_MIN;
	gcode.registers[N].value.d = NAN;
	gcode.registers[O].value.d = NAN;
	gcode.registers[P].value.i = INT_MIN;
	gcode.registers[Q].value.d = NAN;
	gcode.registers[R].value.d = NAN;
	gcode.registers[S].value.d = NAN;
	gcode.registers[T].value.i = INT_MIN;
	gcode.registers[U].value.d = NAN;
	gcode.registers[V].value.d = NAN;
	gcode.registers[W].value.d = NAN;
	gcode.registers[X].value.d = NAN;
	gcode.registers[Y].value.d = NAN;
	gcode.registers[Z].value.d = NAN;
}


void
_gcode_error(gcode_error_t error)
{
	gcode.error = error;
	
	// Skip this line
	char c;
	while (xQueuePeek(gcode.queue, &c, portMAX_DELAY)
	       && !(c == '\n' || c == '\r')) {
		xQueueReceive(gcode.queue, &c, portMAX_DELAY);
	}
}


size_t
gcode_interpret(char *code, size_t len)
{
	size_t i;
	for (i = 0; i < len; i++)
		if (xQueueSend(gcode.queue, code++, (portTickType) 0) == errQUEUE_FULL)
			break;
	return i;
}


size_t
gcode_queue_length(void)
{
	return uxQueueMessagesWaiting(gcode.queue);
}


size_t
gcode_queue_space(void)
{
	return GCODE_BUFFER_LENGTH - gcode_queue_length();
}


int
gcode_instructions_handled(void)
{
	return gcode.instructions_handled;
}


void
gcode_reset_counter(void)
{
	gcode.instructions_handled = 0;
}


void
gcode_task(void *pvParameters)
{
	bool line_empty = true;
	
	for (;;) {
		char c;
		xQueuePeek(gcode.queue, &c, portMAX_DELAY);
		
		switch (c) {
			// Skip whitespace
			case ' ':
			case '\t':
				xQueueReceive(gcode.queue, &c, portMAX_DELAY);
				break;
			
			// Ignore line comments
			case '/':
			case ';':
				_gcode_ignore_line();
				break;
			
			// Ignore block comments
			case '(': 
				_gcode_ignore_until(')');
				break;
			
			// End-of-line
			case '\n':
			case '\r':
				xQueueReceive(gcode.queue, &c, portMAX_DELAY);
				
				// If the line wasn't empty, process the system registers
				if (!line_empty) {
					_gcode_step();
					gcode.instructions_handled ++;
					line_empty = true;
				}
				break;
			
			default:
				if (c >= 'A' && c <= 'Z') {
					xQueueReceive(gcode.queue, &c, portMAX_DELAY);
					c -= 'A';
					if ((gcode.registers[c].is_integer
					     && _gcode_read_int(&(gcode.registers[c].value.i)))
					   ||
					    (!gcode.registers[c].is_integer
					     && _gcode_read_double(&(gcode.registers[c].value.d)))) {
						line_empty = false;
					} else {
						_gcode_error(GCODE_ERR_INVALID_VALUE);
						line_empty = true;
					}
				} else {
					_gcode_error(GCODE_ERR_INVALID_REG);
					line_empty = true;
				}
				break;
		}
	}
}


void
_gcode_ignore_line(void)
{
	char c;
	
	// Scan until an EOL char
	while (xQueuePeek(gcode.queue, &c, portMAX_DELAY)
	       && c != '\n'
	       && c != '\r')
		xQueueReceive(gcode.queue, &c, portMAX_DELAY);
}


void
_gcode_ignore_until(char m)
{
	char c;
	
	// Scan until (and including) the requested char is read
	while (xQueueReceive(gcode.queue, &c, portMAX_DELAY)
	       && c != m)
		;
}


bool
_gcode_read_int(int *value)
{
	char buf[GCODE_VALUE_BUF_SIZE];
	char *cursor = buf;
	
	// Check the buffer's not full and read until the end of the number
	while (((cursor - buf) < GCODE_VALUE_BUF_SIZE - 1)
	       && xQueuePeek(gcode.queue, cursor, portMAX_DELAY)
	       && (isdigit(*cursor)
	           || (*cursor) == '+'
	           || (*cursor) == '-')) {
		xQueueReceive(gcode.queue, cursor, portMAX_DELAY);
		cursor++;
	}
	
	// Have we read some digits?
	if (buf == cursor)
		return false;
	
	// Null-terminate
	*cursor = '\0';
	
	// Decode the integer
	*value = atoi(buf);
	return true;
}


bool
_gcode_read_double(double *value)
{
	char buf[GCODE_VALUE_BUF_SIZE];
	char *cursor = buf;
	
	// Check the buffer's not full and read until the end of the number
	while (((cursor - buf) < GCODE_VALUE_BUF_SIZE - 1)
	       && xQueuePeek(gcode.queue, cursor, portMAX_DELAY)
	       && (isdigit(*cursor)
	           || (*cursor) == '.'
	           || (*cursor) == '+'
	           || (*cursor) == '-')) {
		xQueueReceive(gcode.queue, cursor, portMAX_DELAY);
		cursor++;
	}
	
	// Have we read some digits?
	if (buf == cursor)
		return false;
	
	// Null-terminate
	*cursor = '\0';
	
	// Decode the double
	*value = atof(buf);
	return true;
}


void
_gcode_step(void)
{
	if (gcode.registers[G].value.i != INT_MIN)
		_gcode_step_g();
	else if (gcode.registers[M].value.i != INT_MIN)
		_gcode_step_m();
	else
		gcode.error = GCODE_ERR_COMMAND_MISSING;
	
	gcode.registers[G].value.i = INT_MIN;
	gcode.registers[M].value.i = INT_MIN;
}


void
_gcode_step_g(void)
{
	switch (gcode.registers[G].value.i) {
		// Movement
		case 0:
		case 1:
			// Deal with unspecified registers
			if (isnan(gcode.registers[X].value.d))
				gcode.registers[X].value.d = makerbot_get_position(0);
			if (isnan(gcode.registers[Y].value.d))
				gcode.registers[Y].value.d = makerbot_get_position(1);
			if (isnan(gcode.registers[Z].value.d))
				gcode.registers[Z].value.d = makerbot_get_position(2);
			if (isnan(gcode.registers[F].value.d))
				gcode.registers[F].value.d = 0.0;
			
			// Move to position at speed at mm/s (not mm/min)
			makerbot_move_to((double[3]){
			                   gcode.registers[X].value.d / gcode.unit_per_mm,
			                   gcode.registers[Y].value.d / gcode.unit_per_mm,
			                   gcode.registers[Z].value.d / gcode.unit_per_mm
			                 },
			                 (gcode.registers[F].value.d / gcode.unit_per_mm) / 60.0);
			break;
		
		// Sleep
		case 4:
			// Deal with unspecified registers
			if (gcode.registers[P].value.i == INT_MIN)
				gcode.registers[P].value.i = 1000;
			
			makerbot_sleep(gcode.registers[P].value.i);
			break;
		
		// Set units used G20: inches, G21: mm
		case 20: gcode.unit_per_mm = GCODE_IN_PER_MM; break;
		case 21: gcode.unit_per_mm = 1.0;             break;
		
		// Positioning style G90: Absolute, G91: Relative
		case 90: break;
		case 91: gcode.error = GCODE_ERR_RELATIVE; break;
		
		// Set origin
		case 92:
			// Deal with unspecified registers
			if (isnan(gcode.registers[X].value.d))
				gcode.registers[X].value.d = makerbot_get_position(0);
			if (isnan(gcode.registers[Y].value.d))
				gcode.registers[Y].value.d = makerbot_get_position(1);
			if (isnan(gcode.registers[Z].value.d))
				gcode.registers[Z].value.d = makerbot_get_position(2);
			
			// Move to position at speed at mm/s (not mm/min)
			makerbot_set_origin((double[3]){
			                      gcode.registers[X].value.d / gcode.unit_per_mm,
			                      gcode.registers[Y].value.d / gcode.unit_per_mm,
			                      gcode.registers[Z].value.d / gcode.unit_per_mm
			                    });
			break;
		
		default:
			gcode.error = GCODE_ERR_UNKNOWN_GCODE;
			break;
	}
}


void
_gcode_step_m(void)
{
	switch (gcode.registers[M].value.i) {
		// PSU Power on/off
		case -1:
			makerbot_set_power(true);
			break;
		case 0:
			makerbot_set_power(false);
			break;
		
		// Wait for heaters
		case 6:
			makerbot_wait_heaters();
			break;
		
		// Power on/off steppers
		case 17:
			makerbot_set_axes_enabled(true);
			break;
		case 18:
			makerbot_set_axes_enabled(false);
			break;
		
		// Extruder-Forward/Backward/Off
		case 101:
			makerbot_set_extruder(true);
			break;
		case 102:
			gcode.error = GCODE_ERR_EXTRUDER_REVERSE;
			// Fall through
		case 103:
			makerbot_set_extruder(false);
			break;
		
		// Set extruder temperature
		case 104:
			// Deal with unspecified registers
			if (isnan(gcode.registers[S].value.d))
				gcode.registers[S].value.d = 0.0;
			
			makerbot_set_temperature(0, gcode.registers[S].value.d);
			break;
		
		// Conveyer on/off
		case 106:
			makerbot_set_platform(true);
			break;
		case 107:
			makerbot_set_platform(false);
			break;
		
		// Set extruder speed
		case 108:
			// Deal with unspecified registers
			if (isnan(gcode.registers[S].value.d))
				gcode.registers[S].value.d = 0.0;
			if (gcode.registers[S].value.d != 255.0)
				gcode.error = GCODE_ERR_EXTRUDER_SPEED;
			break;
		
		// Set platform temperature
		case 109:
			// Deal with unspecified registers
			if (isnan(gcode.registers[S].value.d))
				gcode.registers[S].value.d = 0.0;
			
			makerbot_set_temperature(1, gcode.registers[S].value.d);
			break;
		
		default:
			gcode.error = GCODE_ERR_UNKNOWN_MCODE;
			break;
	}
}
