#ifndef NETWORK_DEBUG_H
#define NETWORK_DEBUG_H

#include <string.h>

#include "MakebedConfig.h"
#include "network.h"

/**
 * Initialise the network debug service.
 */
void network_debug_init(void);

/**
 * Get a string to write into for debugging purposes
 */
char *network_debug_str();

void network_debug_appcall(void);
PT_THREAD(network_debug_thread)(network_debug_t *s);

#endif

