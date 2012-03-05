#ifndef NETWORK_UIP_STATE
#define NETWORK_UIP_STATE

#include "psock.h"
#include <stddef.h>
#include <stdbool.h>

/* Buffer size of gcode network buffer */
#define NETWORK_DEBUG_BUFFER_SIZE 256

/**
 * State of the debug interface
 */
typedef struct {
	struct psock p;
	
	// Buffer to store debug message
	char buf[NETWORK_DEBUG_BUFFER_SIZE];
} network_debug_t;

/**
 * State of a gcode processing connection.
 */
typedef struct {
	
} network_gcode_t;

/**
 * State of a UDP gcode processing connection.
 */
typedef struct {
	
} network_udp_gcode_t;


/**
 * Connection type of a connection state.
 */
typedef enum {
	NETWORK_CON_GCODE,
	NETWORK_CON_DEBUG,
	NETWORK_CON_UNKNOWN,
} network_con_type_t;


/**
 * Connection type of a UDP state.
 */
typedef enum {
	NETWORK_UDP_GCODE,
	NETWORK_UDP_UNKNOWN,
} network_udp_type_t;


/**
 * uIP appstate data structure to store application specific connection state
 * data.
 */
typedef struct {
	// Data structures for each connection type
	union {
		network_gcode_t gcode;
		network_debug_t debug;
	} state;
	
	network_con_type_t type;
} uip_tcp_appstate_t;


/**
 * uIP appstate data structure to store application specific connection state
 * data for UDP.
 */
typedef struct {
	// Data structures for each connection type
	union {
		network_udp_gcode_t gcode;
	} state;
} uip_udp_appstate_t;


void network_init(void);
void network_appcall(void);

#define UIP_APPCALL network_appcall
#define UIP_UDP_APPCALL network_udp_appcall

#endif
