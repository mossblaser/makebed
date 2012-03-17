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
		// Reset the gcode counter
		gcode_reset_counter();
		makerbot_reset_underruns();
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
		// Get the seq number
		uint32_t seq_num;
		uint32_t windowsize;
		memcpy(&seq_num, uip_appdata, sizeof(uint32_t));
		
		// Is this an expected sequence number?
		if (seq_num == 0 /* Start */
		    || seq_num == state->seq_num /* Retransmission */
		    || seq_num == state->seq_num + 1 /* Next Packet */) {
			// Reset the gcode counter
			if (seq_num == 0) {
				gcode_reset_counter();
				makerbot_reset_underruns();
			}
			
			// Only interpret the data if its new
			if (seq_num != state->seq_num)
				gcode_interpret(uip_appdata + sizeof(uint32_t), uip_datalen() - sizeof(uint32_t));
			
			state->seq_num = seq_num;
			
			// How big is the window?
			windowsize = gcode_queue_space();
			if (windowsize > UIP_BUFSIZE - 96)
				windowsize = UIP_BUFSIZE - 96;
		} else {
			// Unexpected sequence number. Send back error (zero window, zero seq) and
			// discard data.
			seq_num = 0;
			windowsize = 0;
		}
		
		// Report back
		memcpy(uip_appdata, &seq_num, sizeof(uint32_t));
		memcpy(uip_appdata + sizeof(uint32_t), &windowsize, sizeof(uint32_t));
		uip_udp_send(sizeof(uint32_t) * 2);
	}
}

