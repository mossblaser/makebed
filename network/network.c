#include "network.h"


void
network_init(void)
{
	// Listen for gcode connections
	uip_listen(HTONS(NETWORK_PORT_GCODE));
}

void
network_appcall(void)
{
	// Get the appstate
	uip_tcp_appstate_t *s = &(uip_conn->appstate);
	
	// New connection
	if (uip_connected()) {
		switch (uip_conn->lport) {
			case HTONS(NETWORK_PORT_GCODE):
				// GCode Port
				s->type = NETWORK_CON_GCODE;
				break;
			
			default:
				// Unknown port
				s->type = NETWORK_CON_UNKNOWN;
				break;
		}
	}
	
	switch (s->type) {
		case NETWORK_CON_GCODE:
			network_gcode_appcall();
			break;
		
		default:
			// Unknown connection type... Do nothing
			break;
	}
}

