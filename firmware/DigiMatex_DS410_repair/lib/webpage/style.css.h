#ifndef INC_STYLE_CSS_H_
#define INC_STYLE_CSS_H_

#include <Arduino.h>

const char PAGE_style_CSS[] PROGMEM = R"=====(
section {
    margin: 1.5em;
    border: 1px solid black;
    padding: 1em;
}

section>h2 {
    background-color: white;
    margin-top: -1.5em;
    width: max-content;
    padding: 0 0.5em;
    margin-left: 0.5em;
}

label {
    display: inline-block;
    width: 180px;
}

input {
    display: inline-block;
    width: 200px;
}

input[type=checkbox] {
    width: auto;
    margin-left: 0;
}

select {
    display: inline-block;
    width: 208px;
    padding-left: 0;
    padding-right: 0;
}

input[type=text]:disabled {
    color: black;
}

input[type=number]:disabled {
    color: black;
}

table, th, td {
    border-collapse: collapse;
}

th {
    background-color: #888888;
    padding-right: 10px;
    padding-left: 10px;
    text-align: left;
}

td {
    padding-right: 10px;
    padding-left: 10px;
    text-align: left;
}

tr:nth-child(odd) {
    background-color: #CCCCCC;
}

tr:hover {
    background-color: yellow;
})=====";
#endif