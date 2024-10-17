/* config.h */

#ifndef CONFIG_H
#define CONFIG_H

#include "cncvis.h"
#include "assembly.h"
#include "actor.h"
#include "light.h"
#include "utils.h"

#include "mxml/mxml.h"

// Function to load configuration from a tab-delimited file
int loadConfiguration(const char *filename, ucncAssembly **rootAssembly, ucncLight ***lights, int *lightCount);

#endif // CONFIG_H
