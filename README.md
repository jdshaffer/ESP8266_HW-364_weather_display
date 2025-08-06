# Weather Display Program for ESP8266 (HW-364a)
This is a simple program that grabs the current weather conditions every 15 minutes and displays it on the OLED display of an HW-364a, which is really just an ESP8266 development board with built-in 128x64 resolution 0.96" OLED display.

![AHT20_BMP280 logo](https://github.com/jdshaffer/display_weather_on_HW-364a/blob/main/weather_display.jpg)

I wanted to share this sample program I created (with help from Google Gemini) as I couldn't find many examples online to help me get started with this development board.

Here are some detailed steps to help you get it up and running on your HW-364a device:

1. Install Arduino IDE


2. Add the "Generic ESP8266 Board" to the IDE 
	1. Open "Settings"
	2. In the Settings window, locate the "Additional Boards Manager URLs" field. 
	3. Add the following URL to this field: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`
	4. Click "OK" to close the Preferences window.
	5. Go to Tools > Board > Boards Manager....
	6. In the Boards Manager, type "ESP8266" into the search bar. 
	7. Find the entry for  "ESP8266 by ESP8266 Community", click on it, and then click the "Install" button to install the latest version of the ESP8266 core for Arduino.
	8. After the installation is complete, go to Tools > Board and scroll down to the "ESP8266 Boards" section.
	9. Select "Generic ESP8266 Board."

3. Double-check that these settings in the Arduino IDE are correct:
	1. Board: "Generic ESP8266 Module"
	2. Port: Ensure the correct COM port is selected.
	3. Upload Speed: "115200"
	4. Flash Size: "1MB (FS:64KB OTA:~470KB)"
	5. Erase Flash: "Only Sktech"

4. Install the necessary libraries into Arduino IDE (via Tools > Manage Libraries...)
	1. ESP8266 Board Support Package
	2. ESP8266WiFi
	3. ESP8266HTTPClient
	4. ArduinoJson

5. And lastly, update the program itself: 
	1. Enter your WiFi SSID and Password in the code
	2. Change the latitude and longitude in the weather URL to your current location (and perhaps change it away from the JP model. See Open-Meteo.com for more info.)
	3. Rename the file from `current_weather.cpp` to `current_weather.ino`
	4. Open `current_weather.ino` in Arduino IDE
	5. Send to your HW-364a development board
