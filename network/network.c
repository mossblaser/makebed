#include "network.h"


void
network_init(void)
{
	uip_listen(HTONS(NETWORK_PORT_GCODE));
	uip_listen(HTONS(NETWORK_PORT_DEBUG));
	
	// Listen on a UDP port
	uip_ipaddr_t addr;
	struct uip_udp_conn *c;
	uip_ipaddr(&addr, 0,0,0,0);
	c = uip_udp_new(&addr, HTONS(0));
	if(c != NULL)
		uip_udp_bind(c, HTONS(NETWORK_PORT_GCODE));
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
			
			case HTONS(NETWORK_PORT_DEBUG):
				// Debugger Port
				s->type = NETWORK_CON_DEBUG;
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
		
		case NETWORK_CON_DEBUG:
			network_debug_appcall();
			break;
		
		default:
			// Unknown connection type... Do nothing
			break;
	}
}

void
network_udp_appcall(void)
{
	// XXX
	network_udp_gcode_appcall();
	
	// Get the appstate
	uip_udp_appstate_t *s = &(uip_udp_conn->appstate);
	
	switch (uip_conn->lport) {
			case HTONS(NETWORK_PORT_GCODE):
			network_udp_gcode_appcall();
			break;
		
		default:
			// Unknown connection type... Do nothing
			break;
	}
}

