#ifndef INC_DEBUG_PRINT_H
#define INC_DEBUG_PRINT_H

#include <Arduino.h>

#if (DEBUG_MESSAGES == 1)
#define debugLog(x) 	Serial.print(x)
#define debugLogln(x)	Serial.println(x)
#else
#define debugLog(x)
#define debugLogln(x)
#endif

#endif