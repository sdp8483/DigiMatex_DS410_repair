var webSocket;
            
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
    wsSend(ACTION_INIT_DATA);
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
                case ACTION_INIT_DATA:
                    parseInitData(data);
                    break;
                case ACTION_SCALE_VALUE:
                    document.getElementById("raw").value = data.raw;
                    document.getElementById("weight").value = data.weight;
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

    document.getElementById("cal").value = data.cal;
    document.getElementById("factor").value = data.factor;
    document.getElementById("offset").value = data.offset;

    document.getElementById("fw_info").value = data.version + " " + data.compile_date + " " + data.compile_time;
    document.getElementById("ssid").value = data.ssid;
}

function wsSend(a) {
    if (a === undefined) {
        console.log("action was undefined");
    } else {
        var data = {action: a};
        const array = [];

        switch (a) {
            case ACTION_SSID_RESET:
                console.log("Resetting SSID to default");
                break;
            case ACTION_PAUSE_STREAM:
                console.log("Pausing ws");
                break;
            case ACTION_RESUME_STREAM:
                console.log("Resuming ws");
                break;
            case ACTION_INIT_DATA:
                console.log("Requesting Init Data");
                break;
            case ACTION_SETTINGS_UPDATE:
                console.log("Updating settings")
                data.units = document.getElementById("units").value;
                data.avg = document.getElementById("avg").value;
                data.brightness = document.getElementById("brightness").value;
                data.cal = document.getElementById("cal").value;
                break;
            case ACTION_TARE:
                console.log("taring");
                break;
            case ACTION_CALIBRATE:
                console.log("calibrating");
                data.cal = document.getElementById("cal").value;
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
    var ssid = document.getElementById("ssid").value;
    var data;

    if((ssid !== null) && (ssid.length != 0)) {
        data = JSON.stringify({'action': ACTION_SSID_UPDATE,
                               'ssid': ssid});
        console.log("Sending: " + data);
        webSocket.send(data);

        alert("Controller disconnected and restarting to update SSID. \nReconnect using new SSID.");
    } else {
        alert("SSID invalid, try again.");
    }
}