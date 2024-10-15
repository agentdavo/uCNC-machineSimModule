/* config.h */

#ifndef CONFIG_H
#define CONFIG_H

#include "api.h"
#include "assembly.h"
#include "light.h"

// Function to load configuration from a tab-delimited file
int loadConfiguration(const char *filename, ucncAssembly **rootAssembly, ucncLight ***lights, int *lightCount);

#endif // CONFIG_H
