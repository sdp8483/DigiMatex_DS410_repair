#ifndef INC_STYLE_CSS_H_
#define INC_STYLE_CSS_H_

#include <Arduino.h>

const char PAGE_style_CSS[] PROGMEM = R"=====(
/* DigiMatex DS410 Style 1.0 March 2022 - Sam Perry */

/* section box style from github.com/atc1441/ESP32_nRF52_SWD */
section {margin: 1.5em;border: 1px solid black; padding: 1em;}
section>h2 {background-color: white; margin-top: -1.5em; width: max-content; padding: 0 0.5em; margin-left: 0.5em;}

/* input style with fixed width */
label {display: inline-block; width: 180px;}
input {display: inline-block; width: 200px; font-size: 14px; border-style: groove; border-width: 2px; border-radius: 4px; border-color: lightgray; padding: 1px 2px 1px 2px;}
input:invalid {border: red solid 3px;}
input[type=submit] {margin-left: 184px; width: 208px;}

/* Output style similar to input */
output {display: inline-block; width: 200px; font-size: 14px; border-style: groove; border-width: 2px; border-radius: 4px; border-color: lightgray; padding: 1px 2px 1px 2px;}
output:empty::before {content: "\200b";   /* unicode zero width space character */}

/* fixed width select */
select {display: inline-block; width: 208px; padding-left: 0; padding-right: 0;}

/* button placed inline with input or output */
button.rightButton {width: 86px;}
button.halfWidth {width: 102px;}

/* button to submit settings */
button.submit-action {margin-left: 184px; width: 208px;}

/* control buttons */
button.control {width: 208px; display: block; margin-bottom: 5px;}
button.control-right {width: 208px; display: block; margin-bottom: 5px; margin-left: 184px;})=====";
#endif