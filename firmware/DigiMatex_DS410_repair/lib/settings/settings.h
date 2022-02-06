#ifndef INC_SETTINGS_H
#define INC_SETTINGS_H

#include <Arduino.h>

/* Defines ------------------------------------------------------------------- */
#define LOADCELL_DOUT_PIN   33
#define LOADCELL_SCK_PIN    32

#define BKP_NAMESPACE		"bkp"			/* namespace where settings are stored */
#define BKP_SSID_ADDR		"ssid"			/* keyword where SSID string is stored */

/* Macros -------------------------------------------------------------------- */

/* typedef ------------------------------------------------------------------- */

typedef struct __settings_handle_t {

} Settings_Handle_t;

/* Globals ------------------------------------------------------------------- */

/* External Globals ---------------------------------------------------------- */

/* Function Prototypes ------------------------------------------------------- */

/* Class --------------------------------------------------------------------- */

#endif