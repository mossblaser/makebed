#ifndef NETWORK_UIP_STATE
#define NETWORK_UIP_STATE

#include "psock.h"
#include <stddef.h>

/* Buffer size of gcode network buffer */
#define NETWORK_GCODE_BUFFER_SIZE 32

/**
 * State of a gcode processing connection.
 */
typedef struct {
	// Buffer to store incoming data
	char buf[UIP_CONF_BUFFER_SIZE];
	
	// Start and end of useful data in our buffer
	char *start;
	size_t len;
} network_gcode_t;


/**
 * Connection type of a connection state.
 */
typedef enum {
	NETWORK_CON_GCODE,
	NETWORK_CON_UNKNOWN,
} network_con_type_t;


/**
 * uIP appstate data structure to store application specific connection state
 * data.
 */
typedef struct {
	// Data structures for each connection type
	union {
		network_gcode_t gcode;
	} state;
	
	network_con_type_t type;
} uip_tcp_appstate_t;


void network_init(void);
void network_appcall(void);

#define UIP_APPCALL network_appcall

#endif
