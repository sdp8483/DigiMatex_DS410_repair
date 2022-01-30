#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <TaskScheduler.h>

#include "LedControl.h"                   /* MAX7219 7-segment Display -------*/
#include "HX711.h"                        /* Weight Scale IC */

#include "settings.h"

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
#define DEVICE_NAME			"DigiMatex DS410"
/* Version should be interpreted as: (MAIN).(TOPIC).(FUNCTION).(BUGFIX)
 * 		MAIN marks major milestones of the project such as release
 * 		TOPIC marks the introduction of a new functionality or major changes
 * 		FUNCTION marks introduction of new functionality and aim to advance the current TOPIC
 * 		BUGFIX marks very minor updates such as bug fix, optimization, or text edit
 */
#define HW_VERSION			"V1.0"
#define FW_VERSION			"V0.1.0.0"

/* WiFi Settings -------------------------------------------------------------*/
#define SSID_LEN			32				        /* sources suggest SSID length has a max of 32 characters */
String ssid_str = DEVICE_NAME + String(" ");

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

#define WS_TRANSMIT_RATE_ms	500
StaticJsonDocument<1028> wsData;

/* Private macro -------------------------------------------------------------*/
#if DEBUG_MESSAGES
#define debugLog(x) 	Serial.print(x)
#define debugLogln(x)	Serial.println(x)
#else
#define debugLog(x)
#define debugLogln(x)
#endif

/* Private variables ---------------------------------------------------------*/
Preferences bkp;							          /* non volatile settings storage */
Settings_Handle_t hs;						        /* setttings handle */
LedControl lc=LedControl(MOSI, SCK, SS);/* MAX7219 7-segment */
HX711 scale;
unsigned long delaytime=250;

/* Private function prototypes -----------------------------------------------*/
void writeArduinoOn7Segment(void);
void scrollDigits(void);

/* Webserver Handles ---------------------------------------------------------*/
void handleIndex(AsyncWebServerRequest *request);
void handleIndexJS(AsyncWebServerRequest *request);
void handleCSS(AsyncWebServerRequest * request);
void handleNotFound(AsyncWebServerRequest *request);
void handleInvalidRequest(AsyncWebServerRequest *request);
void onWsEvent(AsyncWebSocket *server, 
			   AsyncWebSocketClient *client, 
			   AwsEventType type, 
			   void *arg, 
			   uint8_t *data, 
			   size_t len);
void wsReceiveParse(uint8_t *data, size_t len);
void wsStream();

/* Task Scheduler ------------------------------------------------------------*/
Scheduler runner;
Task wsTask(WS_TRANSMIT_RATE_ms, TASK_FOREVER, &wsStream);

void setup() {
	Serial.begin(115200);

	/* Get settings from EEPROM or use defaults if not set */
	bkp.begin(BKP_NAMESPACE);

	/* setup task scheduler */
	runner.init();

  /* 7-segment */
  /*
  The MAX72XX is in power-saving mode on startup,
  we have to do a wakeup call
  */
  lc.shutdown(0,false);
  /* Set the brightness to a medium values */
  lc.setIntensity(0,8);
  /* and clear the display */
  lc.clearDisplay(0);

  /* scale */
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

	/* setup default SSID with chip ID */
	ssid_str.reserve(SSID_LEN);
	uint32_t chipId = 0;
	for (uint8_t i=0; i<17; i+=8) {
		chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
	ssid_str += String(chipId, HEX);

	/* retreive user set SSID if avaliable, use above if not */
	ssid_str = bkp.getString(BKP_SSID_ADDR, ssid_str);
	bkp.end();

	/* start WiFi ap -------------------- */
	debugLogln(F("Setting AP ..."));
	WiFi.softAP(ssid_str.c_str());
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
}

void loop() {
	runner.execute();
	ws.cleanupClients();

  writeArduinoOn7Segment();
  scrollDigits();
}

void writeArduinoOn7Segment() {
  lc.setChar(0,6,'a',false);
  delay(delaytime);
  lc.setRow(0,5,0x05);
  delay(delaytime);
  lc.setChar(0,4,'d',false);
  delay(delaytime);
  lc.setRow(0,3,0x1c);
  delay(delaytime);
  lc.setRow(0,2,B00010000);
  delay(delaytime);
  lc.setRow(0,1,0x15);
  delay(delaytime);
  lc.setRow(0,0,0x1D);
  delay(delaytime);
  lc.clearDisplay(0);
  delay(delaytime);
}

void scrollDigits() {
  for(int i=0;i<13;i++) {
    lc.setDigit(0,3,i,false);
    lc.setDigit(0,2,i+1,false);
    lc.setDigit(0,1,i+2,false);
    lc.setDigit(0,0,i+3,false);
    delay(delaytime);
  }
  lc.clearDisplay(0);
  delay(delaytime);
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
			runner.addTask(wsTask);
			wsTask.enable();
			break;
		case WS_EVT_DISCONNECT:
			/* client disconnected */
			Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
			wsTask.disable();
			runner.deleteTask(wsTask);
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
					wsReceiveParse(data, info->len);
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

void wsReceiveParse(uint8_t *data, size_t len) {
	/* get data from JSON */
	DeserializationError error = deserializeJson(wsData, data, len);

	if (error) {
		debugLog("deserializeJason() failed: ");
		debugLogln(error.f_str());
	}

	switch (wsData["action"].as<uint8_t>()) {
	case ACTION_SSID_UPDATE:			/* update ssid */
		bkp.begin(BKP_NAMESPACE, false);
		ssid_str = wsData["ssid"].as<String>();
		bkp.putString(BKP_SSID_ADDR, ssid_str);
		bkp.end();
		ESP.restart();
		break;
	default:
		break;
	}
}

/* Task Schedular Functions -------------------------------------------------- */
void wsStream() {
	StaticJsonDocument<2048> data;

	data["compile_date"] 	= __DATE__;
	data["compile_time"] 	= __TIME__;
	data["version"] 		= FW_VERSION;
	data["ssid"] 			= ssid_str;
	data["millis"] 			= millis();

	String json;
	serializeJson(data, json);

	ws.textAll(json.c_str(), json.length());
}