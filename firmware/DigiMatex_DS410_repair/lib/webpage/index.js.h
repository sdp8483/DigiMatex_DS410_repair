#ifndef INC_INDEX_JS_H_
#define INC_INDEX_JS_H_

#include <Arduino.h>

const char PAGE_index_JS[] PROGMEM = R"=====(
var webSocket;
var wsCount;
var esp32_millis;
            
function initIndex() {
    /* connect to websocket server */
    webSocket = new WebSocket('ws://' + window.location.hostname + '/ws');

    /* assign callbacks */
    webSocket.onopen = function(event) {wsOpen(event)};
    webSocket.onclose = function(event) {wsClose(event)};
    webSocket.onmessage = function(event) {wsReceive(event)};
    webSocket.onerror = function(event) {wsError(event)};

    wsCount = 0;
}

function wsOpen(event) {
    console.log("Connected");
}

function wsClose(event) {
    console.log("Disconnected");

    /* try to reconnect after a few seconds */
    setTimeout(function() {initIndex()}, 2000);
}

function wsError(event) {
    console.log("ERROR: " + event.data);
}

function wsReceive(event) {
    if ((event.data != null) && (event.data != undefined) && (event.data != "")) {
        if (event.data[0] == '{') {
            var data = JSON.parse(event.data);
            esp32_millis = data.millis;

            /* update the input boxes only once */
            if(wsCount < 10) {
                wsCount = wsCount + 1;

                document.getElementById("fw_info").value = data.version + " " + data.compile_date + " " + data.compile_time;
                document.getElementById("ssid").value = data.ssid;
            }
        }
    }
}

function wsSend() {
    var data;

    data = JSON.stringify({ 'action': 1});

    console.log("Sending: " + data);
    webSocket.send(data);

    wsCount = 0;
}

function ssidUpdate() {
    var ssid = document.getElementById("ssid").value;
    var data;

    if(ssid != null) {
        data = JSON.stringify({'action': 0,
                               'ssid': ssid});
        console.log("Sending: " + data);
        webSocket.send(data);

        alert("Controller disconnected and restarting to update SSID. \nReconnect using new SSID.");
    } else {
        alert("SSID invalid, try again.");
    }
})=====";
#endif