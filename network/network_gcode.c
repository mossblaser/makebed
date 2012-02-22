#include "network_gcode.h"


// XXX
#include "gpio.h"

void
network_gcode_appcall(void)
{
	// State of uip connection
	uip_tcp_appstate_t *s = &(uip_conn->appstate);
	
	// State of gcode app
	network_gcode_t *state = &(s->state.gcode);
	
	// Initialise the socket on connect
	if (uip_connected()) {
		state->start = NULL;
		state->len = 0;
		gpio_set_mode(&gpio_mbed_led4, GPIO_OUTPUT);
	}
	
	
	if (uip_newdata()) {
		size_t buffered = gcode_interpret(uip_appdata, uip_datalen());
		
		// If not all data was buffered, we must wait
		if (buffered != uip_datalen()) {
			uip_stop();
			
			// Copy remaining data into a buffer
			state->len = uip_datalen() - buffered;
			strncpy(state->buf, uip_appdata, state->len);
			state->start = state->buf;
		}
	}
	
	// XXX
	if (uip_poll())
		gpio_write(&gpio_mbed_led4, !gpio_read(&gpio_mbed_led4));
	
	// If we stopped the incoming data, resume it once we've dealt with the buffer
	if (uip_poll() && uip_stopped(uip_conn)) {
		if (state->start == NULL)
			uip_abort();
		
		size_t buffered = gcode_interpret(state->start, state->len);
		state->len -= buffered;
		state->start += buffered;
		
		// If the buffer has been empied, continue as usual!
		if (state->len == 0) {
			state->start = NULL;
			uip_restart();
		}
	}
}
