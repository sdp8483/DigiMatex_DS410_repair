#ifndef INC_SETTINGS_H
#define INC_SETTINGS_H

#include <Arduino.h>
#include <Preferences.h>
#include "debug_print.h"

/* Defines ------------------------------------------------------------------- */
#define SCALE_RATE_ms       1000
#define DISPLAY_RATE_ms     500

#define LOADCELL_DOUT_PIN   32
#define LOADCELL_SCK_PIN    33

#define DISPLAY_MOSI_PIN    23
#define DISPLAY_CLK_PIN     18
#define DISPLAY_CS_PIN      17

#define LED_WIFI_PIN        25
#define LED_LBS_PIN         26
#define LED_KG_PIN          27

#define BUTTON_DISPLAY_PIN  19
#define BUTTON_UNIT_PIN     21
#define BUTTON_TARE_PIN     22

#define SSID_LEN            32              /* sources suggest SSID length has a max of 32 characters */

// keywords where settings are saved in eeprom
#define WIFI_NAMESPACE      "wifi"          /* namespace where WiFi settings are stored */
#define AP_SSID		        "ssid"			/* AP SSID string */
#define SCALE_NVS           "scale"         /* scale settings */
#define DISPLAY_NVS         "display"       /* display settings */

/* Macros -------------------------------------------------------------------- */

/* typedef ------------------------------------------------------------------- */
typedef struct __scale_settings_t {
    uint8_t units;                          /* display units */
    uint8_t averaging;                      /* number of samples to average for each scale reading */
    float cal_weight;                       /* weight used for calibration in kg */
    float factor;                           /* scale factor to convert adc readings to kg */
} Scale_Settings_t;

typedef struct __display_settings_t {
    uint8_t intensity;                      /* display brightness 0 to 15 */
} Display_Settings_t;

/* Globals ------------------------------------------------------------------- */

/* External Globals ---------------------------------------------------------- */
extern Preferences bkp;						/* non volatile settings storage */

/* Function Prototypes ------------------------------------------------------- */

/* Class --------------------------------------------------------------------- */
class Settings {
    public:
        // Settings();
        void begin(void);                       /* initialize and get settings from eeprom */

        void getSSID(void);                     /* get AP ssid or use default */
        void setSSID(String new_ssid);          /* set AP ssid to user defined */
        void resetSSID(void);                   /* reset AP ssid to default */

        void getData(const char *name, void *buffer, size_t len);   /* get settings at name from nvs */
        void putData(const char *name, void *buffer, size_t len);   /* put settings at name from nvs */

        /* wifi settings */
        String ssid_str;

        /* scale settings */
        Scale_Settings_t scale;

        /* display settings */
        Display_Settings_t display;

    private:
        uint32_t _getChipID(void);
};

#endif