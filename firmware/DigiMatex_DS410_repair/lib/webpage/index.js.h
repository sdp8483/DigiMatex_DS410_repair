#ifndef INC_INDEX_JS_H_
#define INC_INDEX_JS_H_

#include <Arduino.h>

const char PAGE_index_JS[] PROGMEM = R"=====(
var webSocket;
var dec;
            
function initIndex() {
    /* connect to websocket server */
    webSocket = new WebSocket('ws://' + window.location.hostname + '/ws');

    /* assign callbacks */
    webSocket.onopen = function(event) {wsOpen(event)};
    webSocket.onclose = function(event) {wsClose(event)};
    webSocket.onmessage = function(event) {wsReceive(event)};
    webSocket.onerror = function(event) {wsError(event)};
}

function wsOpen(event) {
    console.log("Connected");

    /* get init data */
    wsSend(2);
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
    if ((event.data !== null) && (event.data !== undefined) && (event.data != "")) {
        if (event.data[0] == '{') {
            console.log("Received JSON data: " + event.data);

            var data = JSON.parse(event.data);

            switch(data.action) {
                case 2:
                    parseInitData(data);
                    break;
                case 6:
                    document.getElementById("raw").value = data.raw;
                    document.getElementById("weight").value = data.weight.toFixed(dec);
                    break;
                default:
                    break;
            }
        }
    }
}

function parseInitData(data) {
    document.getElementById("units").value = data.units;
    document.getElementById("avg").value = data.avg;

    document.getElementById("brightness").value = data.brightness;

    dec = data.dec;
    document.getElementById("dec").value = dec;

    document.getElementById("cal").value = data.cal.toFixed(dec);
    document.getElementById("factor").value = data.factor.toFixed(dec);
    document.getElementById("offset").value = data.offset;

    document.getElementById("fw_info").value = data.version + " " + data.compile_date + " " + data.compile_time;
    document.getElementById("ssi").value = data.ssid;
}

function wsSend(a) {
    if (a === undefined) {
        console.log("action was undefined");
    } else {
        var data = {action: a};
        const array = [];

        switch (a) {
            case 1:
                console.log("Resetting SSID to default");
                break;
            case 3:
                console.log("Pausing ws");
                break;
            case 4:
                console.log("Resuming ws");
                break;
            case 2:
                console.log("Requesting Init Data");
                break;
            case 5:
                console.log("Updating settings")
                data.units = document.getElementById("units").value;
                data.avg = document.getElementById("avg").value;
                data.brightness = document.getElementById("brightness").value;
                data.cal = document.getElementById("cal").value;
                data.dec = document.getElementById("dec").value;
                dec = document.getElementById("dec").value;
                break;
            case 7:
                console.log("taring");
                break;
            case 8:
                console.log("calibrating");
                data.cal = document.getElementById("cal").value;
                data.units = document.getElementById("units").value;
                break;
            default:
                console.log("default");
        }
        var d = JSON.stringify(data);

        console.log("Sending: " + d);
        webSocket.send(d);
    }

    return false;
}

function ssidUpdate() {
    var ssid = document.getElementById("ssi").value;
    var data;

    if((ssid !== null) && (ssid.length != 0)) {
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