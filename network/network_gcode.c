#include "network_gcode.h"


void
network_gcode_appcall(void)
{
	// State of uip connection
	uip_tcp_appstate_t *s = &(uip_conn->appstate);
	
	// State of gcode app
	network_gcode_t *state = &(s->state.gcode);
	
	// Initialise the socket on connect
	if (uip_connected()) {
		// Do nothing
	} else if (uip_stopped(uip_conn)) {
		// If the buffer has room again, continue!!
		if (gcode_queue_space() >= UIP_BUFSIZE)
			uip_restart();
	} else if (uip_newdata()) {
		size_t buffered = gcode_interpret(uip_appdata, uip_datalen());
		
		// If not all data was buffered, then the buffer isn't big enough or there's
		// someone else trying to fill it so abort the connection!
		if (buffered != uip_datalen())
			uip_abort();
		
		// If the buffer hasn't got room for another packet worth, pause
		if (gcode_queue_space() < UIP_BUFSIZE)
			uip_stop();
	}
}

void
network_udp_gcode_appcall(void)
{
	// State of uip connection
	uip_udp_appstate_t *s = &(uip_udp_conn->appstate);
	
	// State of gcode app
	network_udp_gcode_t *state = &(s->state.gcode);
	
	// Accept new data
	if (uip_newdata()) {
		gcode_interpret(uip_appdata, uip_datalen());
		
		// How big is the window?
		uint32_t windowsize = gcode_queue_space();
		if (windowsize > UIP_BUFSIZE - 96)
			windowsize = UIP_BUFSIZE - 96;
		
		// Report back
		memcpy(uip_appdata, &windowsize, sizeof(uint32_t));
		uip_udp_send(sizeof(uint32_t));
	}
}

