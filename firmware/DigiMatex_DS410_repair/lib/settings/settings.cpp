#include "settings.h"

void Settings::begin(void) {
	ssid_str.reserve(SSID_LEN);

	/* get WiFi settings */
	getSSID();

    /* get scale settings */
    getData(SCALE_NVS, &scale, sizeof(scale));

    /* get display settings */
    getData(DISPLAY_NVS, &display, sizeof(display));

}

uint32_t Settings::_getChipID(void) {
	uint32_t chipId = 0;
	for (uint8_t i=0; i<17; i+=8) {
		chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}

	return chipId;
}

void Settings::getSSID(void) {
	bkp.begin(WIFI_NAMESPACE);

	/* assemble default ssid */
	ssid_str = DEVICE_NAME + String(" ");
	ssid_str += String(_getChipID(), HEX);

	debugLog(F("Default SSID: "));
	debugLogln(ssid_str);

	/* get user defined if available */
	ssid_str = bkp.getString(AP_SSID, ssid_str);

	debugLog(F("Stored SSID: "));
	debugLogln(ssid_str);

	bkp.end();
}

void Settings::setSSID(String new_ssid) {
	debugLog(F("New SSID: "));
	debugLogln(new_ssid);

	bkp.begin(WIFI_NAMESPACE);
	bkp.putString(AP_SSID, new_ssid);
	bkp.end();
}

void Settings::resetSSID(void) {
	debugLogln("Reset SSID");

	bkp.begin(WIFI_NAMESPACE);
	bkp.remove(AP_SSID);
	bkp.end();
}

void Settings::getData(const char *name, void *buffer, size_t len) {
	bkp.begin(name);
	bkp.getBytes(name, buffer, len);
	bkp.end();
}

void Settings::putData(const char *name, void *buffer, size_t len) {
	bkp.begin(name);
	bkp.putBytes(name, buffer, len);
	bkp.end();
}