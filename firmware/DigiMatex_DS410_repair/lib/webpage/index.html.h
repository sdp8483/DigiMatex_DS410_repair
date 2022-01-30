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
        <h2>Controller Settings</h2>
        <form>
        <div> 
            <label for="ssid">SSID</label><input type="text" id="ssid" name="ssid" maxlength="30">
            <button type="button" class="rButton" onclick="ssidUpdate()">Update SSID</button>
        </div>
        <div>
            <label for="fw_info">Firmware</label><input type="text" id="fw_info" name="fw_info" disabled="true">
            <button type="button" class="rButton" onclick="window.location.href='/update'">Update FW</button>
        </div>
        </form>
    </section>
</body>
</html>)=====";
#endif