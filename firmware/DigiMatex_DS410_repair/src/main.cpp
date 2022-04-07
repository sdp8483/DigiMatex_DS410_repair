#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <TaskScheduler.h>
#include <Bounce2.h>

#include "LedControl.h"                   /* MAX7219 7-segment Display -------*/
#include "HX711.h"                        /* Weight Scale IC -----------------*/

#include "settings.h"
#include "debug_print.h"

#define ARDUINOJSON_USE_LONG_LONG 1
#include <ArduinoJson.h>

/* ESP32 WiFi Includes -------------------------------------------------------*/
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <AsyncElegantOTA.h>

/* Raw String Literals for Webpages ------------------------------------------*/
#include "index.html.h"
#include "index.js.h"
#include "style.css.h"

/* Private Includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
// #define DEVICE_NAME			"DigiMatex DS410"
/* Version should be interpreted as: (MAIN).(TOPIC).(FUNCTION).(BUGFIX)
 * 		MAIN marks major milestones of the project such as release
 * 		TOPIC marks the introduction of a new functionality or major changes
 * 		FUNCTION marks introduction of new functionality and aim to advance the current TOPIC
 * 		BUGFIX marks very minor updates such as bug fix, optimization, or text edit
 */
#define HW_VERSION			"V1.0"
#define FW_VERSION			"V0.1.0.0"

/* WiFi Settings -------------------------------------------------------------*/
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#define WS_TRANSMIT_RATE_ms	500
StaticJsonDocument<1028> wsData;

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
Preferences bkp;							        /* non volatile settings storage */
Settings settings;									/* all the settings */

LedControl lc=LedControl(DISPLAY_MOSI_PIN, 			/* MAX7219 7-segment */
						 DISPLAY_CLK_PIN, 
						 DISPLAY_CS_PIN);

bool display_sleep = false;

HX711 scale;
long scale_raw;
float scale_units;

#define SCALE_WINDOW_LEN	255
long scale_window[SCALE_WINDOW_LEN];
int16_t scale_window_index = 0;

Bounce2::Button displayButton = Bounce2::Button();
Bounce2::Button tareButton = Bounce2::Button();
Bounce2::Button unitButton = Bounce2::Button();

/* Private function prototypes -----------------------------------------------*/
void displayTest(void);
void displayTare(void);
void updateDisplay(void);
void readButtons(void);
void readScale(void);
void ledWrite(uint8_t led_pin, uint8_t val);

/* Webserver Handles ---------------------------------------------------------*/
void handleIndex(AsyncWebServerRequest *request);
void handleIndexJS(AsyncWebServerRequest *request);
void handleCSS(AsyncWebServerRequest * request);
void handleNotFound(AsyncWebServerRequest *request);
void handleInvalidRequest(AsyncWebServerRequest *request);

/* Websocket functions -------------------------------------------------------*/
void onWsEvent(AsyncWebSocket *server, 
			   AsyncWebSocketClient *client, 
			   AwsEventType type, 
			   void *arg, 
			   uint8_t *data, 
			   size_t len);
void wsReceiveParse(AsyncWebSocketClient *client, uint8_t *data, size_t len);
void wsSendInitData(uint32_t client_id);
void wsStream();

/* Task Scheduler ------------------------------------------------------------*/
Scheduler runner;
Task wsTask(WS_TRANSMIT_RATE_ms, TASK_FOREVER, &wsStream);
Task scaleTask(SCALE_RATE_ms, TASK_FOREVER, &readScale);
Task displayTask(DISPLAY_RATE_ms, TASK_FOREVER, &updateDisplay);

void setup() {
	Serial.begin(115200);

	/* setup LEDs */
	pinMode(LED_WIFI_PIN, OUTPUT);
	pinMode(LED_LBS_PIN, OUTPUT);
	pinMode(LED_KG_PIN, OUTPUT);
	digitalWrite(LED_WIFI_PIN, LOW);
	digitalWrite(LED_LBS_PIN, LOW);
	digitalWrite(LED_KG_PIN, LOW);
	ledcSetup(LED_PWM_CH, LED_FREQ, LED_PWM_RESOLUTION);

	/* setup buttons */
	displayButton.attach(BUTTON_DISPLAY_PIN, INPUT_PULLUP);
	displayButton.interval(BUTTON_DEBOUNCE_ms);
	displayButton.setPressedState(LOW);

	tareButton.attach(BUTTON_TARE_PIN, INPUT_PULLUP);
	tareButton.interval(BUTTON_DEBOUNCE_ms);
	tareButton.setPressedState(LOW);

	unitButton.attach(BUTTON_UNIT_PIN, INPUT_PULLUP);
	unitButton.attach(BUTTON_DEBOUNCE_ms);
	unitButton.setPressedState(LOW);

	/* initialize settings and get from eeprom */
	settings.begin();

	/* setup task scheduler */
	runner.init();

	/* 7-segment */
	lc.shutdown(0, display_sleep);		/* wakeup */
	lc.setIntensity(0, settings.display.intensity);
	lc.clearDisplay(0);

	/* scale */
	scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

	/* start WiFi ap -------------------- */
	debugLogln(F("Setting AP ..."));
	WiFi.softAP(settings.ssid_str.c_str());
	IPAddress IP = WiFi.softAPIP();
	debugLog(F("AP IP address: "));
	debugLogln(IP);

	/* Websocket */
	ws.onEvent(onWsEvent);
	server.addHandler(&ws);

	/* server page request handles */
	server.on("/", HTTP_GET, handleIndex);
	server.on("/index.js", handleIndexJS);
	server.on("/style.css", handleCSS);
	server.onNotFound(handleNotFound);

	AsyncElegantOTA.begin(&server);
	server.begin();

	displayTest();

	displayTare();
	scale.tare(TARE_SAMPLES);
	scale.set_scale(settings.scale.factor);

	/* fill scale buffer */
	long val = scale.read_average(settings.scale.averaging);
	for (uint8_t i=0; i<settings.scale.window; i++) {
		scale_window[i] = val;
	}

	/* start scale reading and display */
	runner.addTask(scaleTask);
	runner.addTask(displayTask);
	scaleTask.enable();
	displayTask.enable();
}

void loop() {
	runner.execute();
	ws.cleanupClients();
	readButtons();
}

/* functions */
void readButtons() {
	displayButton.update();
	tareButton.update();
	unitButton.update();

	if (displayButton.pressed()) {
		display_sleep = !display_sleep;
		lc.shutdown(0, display_sleep);
	}

	if (tareButton.pressed()) {
		displayTare();
		scale.tare(TARE_SAMPLES);
	}

	if (unitButton.pressed()) {
		if (settings.scale.units == UNITS_KG) {
			settings.scale.units = UNITS_LBS;
		} else {
			settings.scale.units = UNITS_KG;
		}
	}
}

void ledWrite(uint8_t led_pin, uint8_t val) {
	if (val == HIGH) {
		ledcAttachPin(led_pin, LED_PWM_CH);
	} else {
		ledcDetachPin(led_pin);
		pinMode(led_pin, OUTPUT);
		digitalWrite(led_pin, LOW);
	}
}

void displayTest() {
	lc.clearDisplay(0);

	ledcWrite(LED_PWM_CH, settings.display.intensity);
	ledWrite(LED_WIFI_PIN, HIGH);
	ledWrite(LED_LBS_PIN, HIGH);
	ledWrite(LED_KG_PIN, HIGH);

	/* Scroll through digits */
	for (int i=0; i<16; i++) {
		for (int d=0; d<8; d++) {
			lc.setDigit(0, d, i, true);
		}
		delay(250);
	}

	ledWrite(LED_WIFI_PIN, LOW);
	ledWrite(LED_LBS_PIN, LOW);
	ledWrite(LED_KG_PIN, LOW);

	lc.clearDisplay(0);
}

void displayTare() {
	lc.clearDisplay(0);
	lc.setRow(0, 5, 0x0f);
	lc.setChar(0, 4, 'a', false);
	lc.setRow(0, 3, 0x05);
	lc.setChar(0, 2, 'e', false);
}

/* Scale Function ------------------------------------------------------------ */
void readScale(void) {
	scale_window[scale_window_index] = scale.read_average(settings.scale.averaging);
	scale_window_index++;

	if (scale_window_index >= settings.scale.window) {
		scale_window_index = 0;
	}

	long sum = 0;
	for (uint8_t i=0; i<settings.scale.window; i++) {
		sum += scale_window[i];
	}

	if (settings.scale.window != 0) {
		scale_raw = sum/settings.scale.window;
	}

	if (settings.scale.units == settings.scale.cal_units) {
		scale_units = (scale_raw - scale.get_offset()) / scale.get_scale();
	} else {
		if (settings.scale.cal_units == UNITS_KG) {
			scale_units = ((scale_raw - scale.get_offset()) / scale.get_scale()) * KG_TO_LBS;
		} else {
			scale_units = ((scale_raw - scale.get_offset()) / scale.get_scale()) * LBS_TO_KG;
		}
	}
}

/* display update ------------------------------------------------------------ */
void updateDisplay(void) {
	static uint8_t units_last = 255;
	static uint8_t softAPStationNum_last = 0;
	uint32_t divisor = 1;
	uint8_t num;
	char sign = ' ';

	if (display_sleep != true) {
		if (units_last != settings.scale.units) {
			units_last = settings.scale.units;
			if (settings.scale.units == UNITS_LBS) {
				ledWrite(LED_LBS_PIN, HIGH);
				ledWrite(LED_KG_PIN, LOW);
			} else {
				ledWrite(LED_LBS_PIN, LOW);
				ledWrite(LED_KG_PIN, HIGH);
			}
		}
	} else {
		ledWrite(LED_LBS_PIN, LOW);
		ledWrite(LED_KG_PIN, LOW);
		units_last = 255;
	}

	/* move over two decimal places */
	int32_t val = (int32_t) (scale_units * (pow(10.0f, settings.display.decimal_places)));

	lc.clearDisplay(0);

	if (val < 0) {
		sign = '-';
	}

	for (int i=0; i<8; i++) {
		if (((abs(val) / divisor) <= 0) && (i>settings.display.decimal_places)) {
			lc.setChar(0, i, sign, false);
			break;
		} else {
			num = (abs(val) / divisor) % 10;
			if (i==settings.display.decimal_places) {
				lc.setDigit(0, i, num, true);	/* set decimal place */
			} else {
				lc.setDigit(0, i, num, false);
			}
			divisor *= 10;
		}
	}

	/* update wifi led */
	if (display_sleep != true) {
		uint8_t clients = WiFi.softAPgetStationNum();
		if (clients != softAPStationNum_last) {
			softAPStationNum_last = clients;
			if (clients > 0) {
				ledWrite(LED_WIFI_PIN, HIGH);
			} else {
				ledWrite(LED_WIFI_PIN, LOW);

				/* stop ws stream when no clients to listen*/
				wsTask.disable();
				runner.deleteTask(wsTask);
			}
		}
	} else {
		ledWrite(LED_WIFI_PIN, LOW);
		softAPStationNum_last = 0;
	}
}

/* Webserver Handlers -------------------------------------------------------- */
void handleIndex(AsyncWebServerRequest *request) {
	request->send_P(200, "text/html", PAGE_index_HTML);
}

void handleIndexJS(AsyncWebServerRequest *request) {
	request->send_P(200, "application/javascript", PAGE_index_JS);
}

void handleCSS(AsyncWebServerRequest *request) {
	request->send_P(200, "text/css", PAGE_style_CSS);
}

void handleNotFound(AsyncWebServerRequest *request) {
	request->send(404, "text/plain", "404: not found");
}

void handleInvalidRequest(AsyncWebServerRequest *request) {
	request->send(400, "text/plain", "400: invalid request");
}

void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, 
			   AwsEventType type, void *arg, uint8_t *data, size_t len) {
	
	AwsFrameInfo * info = (AwsFrameInfo*)arg;
    switch(type) {
		case WS_EVT_CONNECT:
			/* client connected */
			Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    		client->printf("Hello Client %u :)", client->id());
    		client->ping();
			break;
		case WS_EVT_DISCONNECT:
			/* client disconnected */
			Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
			break;
		case WS_EVT_ERROR:
			/* error was received from the other end */
			Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
			break;
		case WS_EVT_PONG:
			/* pong message was received (in response to a ping request maybe) */
			Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
			break;
		case WS_EVT_DATA:
			/* data packet */
			if(info->final && info->index == 0 && info->len == len) {
				//the whole message is in a single frame and we got all of it's data
				Serial.printf("ws[%s][%u] %s-message[%llu]: ", 
							server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
				if(info->opcode == WS_TEXT) {
					data[len] = 0;
					Serial.printf("%s\n", (char*)data);

					wsReceiveParse(client, data, info->len);
				} else {
					for(size_t i=0; i < info->len; i++){
						Serial.printf("%02x ", data[i]);
					}
					Serial.printf("\n");
				}
				if(info->opcode == WS_TEXT)
					client->text("I got your text message");
				else
					client->binary("I got your binary message");
			} else {
				//message is comprised of multiple frames or the frame is split into multiple packets
				if(info->index == 0){
					if(info->num == 0)
					Serial.printf("ws[%s][%u] %s-message start\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
					Serial.printf("ws[%s][%u] frame[%u] start[%llu]\n", server->url(), client->id(), info->num, info->len);
				}

				Serial.printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", 
							server->url(), client->id(), info->num, 
							(info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
				if(info->message_opcode == WS_TEXT){
					data[len] = 0;
					Serial.printf("%s\n", (char*)data);
				} else {
					for(size_t i=0; i < len; i++){
						Serial.printf("%02x ", data[i]);
					}
					Serial.printf("\n");
				}

				if((info->index + len) == info->len){
					Serial.printf("ws[%s][%u] frame[%u] end[%llu]\n", server->url(), client->id(), info->num, info->len);
					if(info->final) {
						Serial.printf("ws[%s][%u] %s-message end\n", 
									server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
						if(info->message_opcode == WS_TEXT)
							client->text("I got your text message");
						else
							client->binary("I got your binary message");
					}
				}
			}
			break;
		default:
			break;
	}
}

void wsReceiveParse(AsyncWebSocketClient *client, uint8_t *data, size_t len) {
	/* get data from JSON */
	DeserializationError error = deserializeJson(wsData, data, len);

	if (error) {
		debugLog("deserializeJason() failed: ");
		debugLogln(error.f_str());
	}

	switch (wsData["action"].as<uint8_t>()) {
		case ACTION_SSID_UPDATE:
			settings.setSSID(wsData["ssid"].as<String>());
			ESP.restart();
			break;
		case ACTION_SSID_RESET:
			settings.resetSSID();
			ESP.restart();
			break;
		case ACTION_INIT_DATA:
			wsSendInitData((uint32_t)client->id());
			break;
		case ACTION_PAUSE_STREAM:
			wsTask.disable();
			runner.deleteTask(wsTask);
			break;
		case ACTION_RESUME_STREAM:
			runner.addTask(wsTask);
			wsTask.enable();
			break;
		case ACTION_SETTINGS_UPDATE:
			settings.scale.units = wsData["units"].as<uint8_t>();
			settings.scale.averaging = wsData["avg"].as<uint8_t>();
			settings.scale.window = wsData["window"].as<uint8_t>();
			settings.scale.cal_weight = wsData["cal"].as<float>();

			settings.display.intensity = wsData["brightness"].as<uint8_t>();
			ledcWrite(LED_PWM_CH, settings.display.intensity);
			lc.setIntensity(0, settings.display.intensity);

			settings.display.decimal_places = wsData["dec"].as<uint8_t>();

			settings.putData(SCALE_NVS, &settings.scale, sizeof(settings.scale));
			settings.putData(DISPLAY_NVS, &settings.display, sizeof(settings.display));
			break;
		case ACTION_TARE:
			displayTare();
			scale.tare(TARE_SAMPLES);
			wsSendInitData((uint32_t)client->id());		/* offset has changed so resend data */
			break;
		case ACTION_CALIBRATE:
			/* reset scale factor */
			settings.scale.factor = 1.0f;
			scale.set_scale(settings.scale.factor);

			/* get calibration weight */
			settings.scale.cal_weight = wsData["cal"].as<float>();
			settings.scale.cal_units = wsData["units"].as<uint8_t>();

			/* calculate new scale factor */
			settings.scale.factor = (scale_raw - scale.get_offset()) / settings.scale.cal_weight;
			scale.set_scale(settings.scale.factor);

			settings.putData(SCALE_NVS, &settings.scale, sizeof(settings.scale));

			wsSendInitData((uint32_t)client->id());		/* scale factor has changed so resend data */
			break;
		case ACTION_DISPLAY:
			display_sleep = !display_sleep;
			lc.shutdown(0, display_sleep);
			break;
		default:
			break;
	}
}

void wsSendInitData(uint32_t client_id) {
	StaticJsonDocument<1024> data;

	data["action"]			= ACTION_INIT_DATA;

	data["compile_date"] 	= __DATE__;
	data["compile_time"] 	= __TIME__;
	data["version"] 		= FW_VERSION;
	data["ssid"] 			= settings.ssid_str;

	data["units"]			= settings.scale.units;
	data["avg"]				= settings.scale.averaging;
	data["window"]			= settings.scale.window;
	data["cal"]				= settings.scale.cal_weight;
	data["factor"]			= settings.scale.factor;
	data["offset"]			= scale.get_offset();

	data["brightness"]		= settings.display.intensity;
	data["dec"]				= settings.display.decimal_places;
	
	String json;
	serializeJson(data, json);

	ws.text(client_id, json.c_str(), json.length());
}

/* Task Schedular Functions -------------------------------------------------- */
void wsStream() {
	StaticJsonDocument<64> data;
	
	data["action"]			= ACTION_SCALE_VALUE;
	data["raw"]				= scale_raw;
	data["weight"]			= scale_units;

	String json;
	serializeJson(data, json);

	ws.textAll(json.c_str(), json.length());
}