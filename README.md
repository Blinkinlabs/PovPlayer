# PovPlayer

![image](https://raw.githubusercontent.com/Blinkinlabs/PovPlayer/master/documentation/DSC_8295.jpg)

POV player is a test tool for displaying POV images. It's based on a Teensy 3.6, and uses a generic SPI touchscreen to configure the POV settings.

A PCB file is included, however the project can be built using short jumper wires if desired.

# Hardware needed

* [Teensy 3.6](https://www.pjrc.com/store/teensy36.html)
* [Generic 2.8" SPI touchscreen](https://www.amazon.com/HiLetgo-240X320-Resolution-Display-ILI9341/dp/B073R7BH1B), using ILI9341 driver
* (optional) PCB made from the [project gerbers](https://github.com/Blinkinlabs/PovPlayer/blob/master/releases/2018-05-29%20POV_Player%20RevA%20Gerber.zip)

# Software Setup

Install:

*  [Arduino](https://www.arduino.cc/)
*  [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html)
*  Adafruit_ILI9341 library for Arduino- this might come with Teensyduino
*  XPT2046_Touchscreen library for Arduino- this might come with Teensyduino

Download or clone this respoitory, then open the [project sketch sketch](https://github.com/Blinkinlabs/PovPlayer/tree/master/SPI_POV) in Arduino and upload it to the Teensy 3.6

# Usage

See the [Usage manual](https://github.com/Blinkinlabs/PovPlayer/raw/master/releases/2018-06-05%20POV%20Player%20Manual%20RevA.pdf)
