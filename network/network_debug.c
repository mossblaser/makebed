#include "network_debug.h"


static char network_debug_message[NETWORK_DEBUG_BUFFER_SIZE] = "Debug.";


void
network_debug_init(void)
{
	network_debug_message[0] = '\0';
}


char *
network_debug_str(void)
{
	return network_debug_message;
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
		strncpy(s->buf, network_debug_message, NETWORK_DEBUG_BUFFER_SIZE);
		PSOCK_SEND_STR(&s->p, s->buf);
	}
	
	PSOCK_END(&s->p);
}

