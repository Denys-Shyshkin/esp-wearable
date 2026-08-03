# ESP32 wearable firmware

An ESP32-C3-based wearable smartwatch designed to apply core embedded systems concepts. This project consolidates my knowledge of electronics fundamentals, microcontroller architecture, and embedded C into a fully functional, integrated system.

## Features

- Time
- Date
- Battery percentage
- Steps count
- Weather forecast
- Heart rate sensor
- Air raid alert map of Ukraine

## Software Stack

- PlatformIO
- ESP-IDF
- Embedded C

## Hardware

| # | Image | Name / Model | Qty | Description |
| --- | --- | --- | --- | --- |
| 1 |  | ESP32-C3 PRO MINI | 1 | MCU based dev board |
| 2 |  | ST7789 LCD TFT  | 1 | 240x240 display |
| 3 |  | BME280 | 1 | environmental sensor |
| 4 |  | QMI8658A | 1 | 6-axis inertial measurement unit  |
| 5 |  | GEB403035 | 1 | LiPo 400 mAh (3.7 V) power supply |
| 6 |  | TP4057 | 1 | charge controller |
| 7 |  | step-up converter | 1 | converts battery input to stable 5V |
| 8 |  | push button | 2 | user input |
| 9 |  | capacitor | 1 | power supply filter |
| 10 |  | resistor | 1 | to create voltage divider |
| 11 |  | resistor | 1 | to create voltage divider |

## System Architecture

*block diagram*

## Wiring

*diagram*

## Schematic

*diagram*

## PCB layout

*diagram*

## Case

*scheme*

## Build Instructions

*step by step guide*

## Memory Usage

*optional*

## Timing

*optional*

## Battery consumption

*table*

## Challenges

*problem -> cause -> solution*

## Future Improvements

- Settings screen (adjust brightness, etc.)

## Screenshots / Photos

*logic analyzer and/or oscilloscope captures*,
*hardware photos*

## Project Progress

- [x]  Display connection
- [x]  Display setup
- [x]  Basic graphic library creation
- [x]  Adding basic font support
- [ ]  Push buttons connection
- [ ]  Introducing state machine
- [ ]  Time screen layout setup
- [ ]  Time update for secs, mins and hours
- [ ]  Weather screen layout setup
- [ ]  Adding special characters support
- [ ]  Adding icons support
- [ ]  Heart rate screen layout setup
- [ ]  Adding animation support
- [ ]  Air raid alerts screen layout setup
- [ ]  Ukraine simplified bitmap output
- [ ]  MUI sensor connection
- [ ]  Wifi connection setup
