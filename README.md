# Sample Programs for HW-364a and HW-364b
This will (hopefully) grow into a small collection of sample programs for the HW-364a and HW-364b development boards, which are really just an ESP8266 development boards with a built-in 128x64 resolution 0.96" OLED display. *(The only difference between HW-364a and HW-364b seems to be the type of USB plug being used: USB-C vs. microUSB, at least that's true as far as I know...)*

![AHT20_BMP280 logo](https://github.com/jdshaffer/display_weather_on_HW-364a/blob/main/weather_display.jpg)

I'm a rather simple guy, and so I will slowly be uploading sample programs with a CPP (C++) extention and a corresponding TXT (text) file with the details on how to use the sample program. Note that you will need to rename each program from `FILENAME.cpp` to `FILENAME.ino` as INO is the extention that Arduino IDE expects to see (even though it's just a CPP file).

To get started, be sure to go through the `How to Get Started.txt` file, which explains how to get the Arduino IDE setup to use with your HW-364a or HW-364b board.

### Sample Programs (each can be found in its own sub-folder)
* `Current Weather` -- Connects to the internet and grabs the current weather conditions for the latitude and longitude specified. Displays the update time, the temperature, the "feels like" temperature, and humidity. 
* 
