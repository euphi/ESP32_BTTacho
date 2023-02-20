# ESP32 Tacho

![Screenshot from FreeCAD](https://github.com/euphi/ESP32_BTTacho/blob/main/Mechanics/Screenshot-FreeCAD-BTTacho2.png)

_See below on information how to compile_

## Features (IN DEVELOPMENT)

* Connect to a classic "Forumslader" (forumslader.de) (5.4 with classic Bluetooth)
* Connect to a BLE Heart Rate Monitor
* Output to 128x64 OLED
* Output to 8x WS2812 strip
* 3D printed case / mount for riserbar

## Todo:

[ ] Touch input for OLED menu
[ ] Get rid of hardcoded BT connections (discovery)
[ ] Robustness (reconnect, ..)
[ ] Streamline OLED display (larger font for speed etc.)
[ ] Support for  Cadence BLE device
[ ] Headlights (some bright LED) as additional "off-road" light

* the 12V supply allow it to add some very bright leds

[ ] 6V bike light output (6V for my BM bike lamps)

* currently it is in parallel to the Forumslader which means the battery can't be charged while light is on

[ ] Main switch

[ ] automatic stopwatch timer (cycling, stopping, resting, ...)


## Ideas

* add gyro
    * alarm system
    * maybe useful to improve statistics, e.g. "lost time due to traffic"
* add bell/sound
    * for alarm system
    * as bell/horn
* add more lights as ambient light (useful when locking your bike at night or at campsite etc.)

## Possible extensions (not in my scope for now):

* adaption to newer variants of Forumslader should be possible
* BLE for other speed measurement

## Goal

* Biketour from Munich to Barcelona in May/June 2022


# Build

To be built with platformio.

If platformio is installed, you can run `platformio run` to build.

However, there are some issues with the Arduino ESP32 framework right now, so

* the configuration in `platformio.ini` is a little bit _weird_
* For now, you need to patch `BluetoothSerial.cpp` in your platformio-packages (`~/.platformio/packages/framework-arduinoespressif32/libraries/BluetoothSerial/src`)
    * Probably I will fork the framework so it is installable automatically
    * see `framework-arduino-esp32-BTSerial.patch`
