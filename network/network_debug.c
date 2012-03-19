#include "network_debug.h"


// General debugging
static char message_dbg[NETWORK_DEBUG_BUFFER_SIZE] = "Debug.";
char *
function_dbg(void)
{
	return message_dbg;
}

// Temperatures
static char message_tmp[NETWORK_DEBUG_BUFFER_SIZE] = "";
char *
function_tmp(void)
{
	sprintf(message_tmp, "%d %d %d %d %d %d\n",
	        (int)(100*makerbot_get_temperature(0)),
	        (int)(100*makerbot_get_set_point(0)),
	        makerbot_get_heater_on(0),
	        (int)(100*makerbot_get_temperature(1)),
	        (int)(100*makerbot_get_set_point(1)),
	        makerbot_get_heater_on(1));
	return message_tmp;
}

// Gcode Stats
static char message_gcd[NETWORK_DEBUG_BUFFER_SIZE] = "";
char *
function_gcd(void)
{
	sprintf(message_gcd, "%d\n", gcode_instructions_handled());
	return message_gcd;
}

// Buffer usage
static char message_buf[NETWORK_DEBUG_BUFFER_SIZE] = "";
char *
function_buf(void)
{
	sprintf(message_buf, "%d %d %d %d %d\n",
	        gcode_queue_length(),
	        GCODE_BUFFER_LENGTH,
	        makerbot_queue_length(),
	        MAKERBOT_COMMAND_QUEUE_LENGTH,
	        makerbot_buffer_underruns());
	return message_buf;
}

// Power Status
static char message_pow[NETWORK_DEBUG_BUFFER_SIZE] = "";
char *
function_pow(void)
{
	sprintf(message_pow, "%d\n", makerbot_get_power());
	return message_pow;
}

// Position of axes
static char message_pos[NETWORK_DEBUG_BUFFER_SIZE] = "";
char *
function_pos(void)
{
	sprintf(message_pos, "%d %d %d\n",
	        (int)(100*makerbot_get_position(0)),
	        (int)(100*makerbot_get_position(1)),
	        (int)(100*makerbot_get_position(2)));
	return message_pos;
}


void
network_debug_init(void)
{
	// Do nothing.
}


char *
network_debug_str(char *key)
{
	if (strcmp(key, "tmp") == 0) return function_tmp();
	if (strcmp(key, "gcd") == 0) return function_gcd();
	if (strcmp(key, "buf") == 0) return function_buf();
	if (strcmp(key, "pow") == 0) return function_pow();
	if (strcmp(key, "pos") == 0) return function_pos();
	
	// If the key is unknown, return the default debug string
	return function_dbg();
}


void
network_debug_appcall(void)
{
	// State of uip connection
	uip_tcp_appstate_t *s = &(uip_conn->appstate);
	
	// State of gcode app
	network_debug_t *state = &(s->state.debug);
	
	// Initialise the socket on connect
	if (uip_connected())
		PSOCK_INIT(&(state->p),
		           state->buf,
		           NETWORK_DEBUG_BUFFER_SIZE);
	
	network_debug_thread(state);
}


PT_THREAD(network_debug_thread)(network_debug_t *s)
{
	PSOCK_BEGIN(&s->p);
	
	while (1) {
		PSOCK_READTO(&s->p, '\n');
		s->buf[3] = '\0';
		strncpy(s->buf, network_debug_str(s->buf), NETWORK_DEBUG_BUFFER_SIZE);
		PSOCK_SEND_STR(&s->p, s->buf);
	}
	
	PSOCK_END(&s->p);
}

