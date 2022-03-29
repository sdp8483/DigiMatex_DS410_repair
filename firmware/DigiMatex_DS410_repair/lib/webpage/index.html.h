#ifndef INC_INDEX_HTML_H_
#define INC_INDEX_HTML_H_

#include <Arduino.h>

const char PAGE_index_HTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta name="description" content="DigiMatex DS410">
    <meta name="author" content="Sam Perry">
    <title>DigiMatex DS410</title>
    <link rel="stylesheet" href="/style.css">
    <link rel="icon" href="data:,"> <!-- disable favicon request -->
    <script src="/index.js"></script>
</head>
<body onload="initIndex()">
    <section>
        <h2>Control</h2>
        <div>
            <label for="raw">Raw Value</label>
            <output id="raw"></output>
            <button class="rightButton" onclick="wsSend(4)">Start</button>
            <button class="rightButton" onclick="wsSend(3)">Stop</button>
        </div>
        <div>
            <label for="weight">Weight</label>
            <output id="weight"></output>
            <button class="rightButton" onclick="wsSend(7)">Tare</button>
        </div>
        <div>
            
        </div>
    </section>
    <section>
        <h2>Settings</h2>
        <div>
            <label for="uints">Units</label>
            <select id="units">
                <option value="0">kilograms</option>
                <option value="1">pounds</option>
            </select>
        </div>
        <div>
            <label for="avg">Averaging</label>
            <input type="number" id="avg" min="1" max="255">
        </div>
        <div>
            <label for="brightness">Display Brightness</label>
            <input type="number" id="brightness" min="0" max="15">
        </div>
        <div>
            <label for="dec">Decimal Places</label>
            <input typ="number" id="dec" min="0" max="6">
        </div>
        <button class="control-right" onclick="wsSend(5)">Update</button>
    </section>

    <section>
        <h2>Calibration</h2>
        <div>
            <label for="cal">Calibration Weight</label>
            <input type="number" id="cal" min="0" max="300" step="0.01">
            <button class="rightButton" onclick="wsSend(8)">Calibrate</button>
        </div>
        <div>
            <label for="factor">Scale Factor</label>
            <output type="number" id="factor"></output>
        </div>
        <div>
            <label for="offset">Scale Offset</label>
            <output type="number" id="offset"></output>
        </div>
    </section>

    <section>
        <h2>About</h2>
        <div> 
            <label for="ssid">SSID</label>
            <input type="text" id="ssid" name="ssid" maxlength="30">
            <button type="button" class="rightButton" onclick="ssidUpdate()">Update SSID</button>
        </div>
        <div>
            <label for="fw_info">Firmware</label>
            <!-- <input type="text" id="fw_info" name="fw_info" disabled="true"> -->
            <output id="fw_info"></output>
            <button type="button" class="rightButton" onclick="window.location.href='/update'">Update FW</button>
        </div>
    </section>
</body>
</html>)=====";
#endif