# Weather Display Program for ESP8266 (HW-364a)
A simple program that grabs the current weather conditions every 15 minutes and displays it on the OLED display of an HW-364a, which is really just an ESP8266 development board with built-in 128x64 resolution 0.96" OLED display.

<<< TO DO -- I will clean up this file as soon as I can to make it easier to understand and follow >>>

I wanted to share this sample program I created (with help from Google Gemini) as I couldn't find many examples online to help me get started with this development board.

IMPORTANT SETTINGS FOR "Arduino IDE"
	Board: "Generic ESP8266 Module"
	Port: Ensure the correct COM port is selected.
	Upload Speed: "115200"
	Flash Size: "1MB (FS:64KB OTA:~470KB)"
	Erase Flash: "Only Sktech"


You will need to add "Generic ESP8266 Board" to the Arduino IDE:
	- Open "Settings"
	- In the Settings window, locate the "Additional Boards Manager URLs" field. 
    Add the following URL to this field: 
        http://arduino.esp8266.com/stable/package_esp8266com_index.json
	- Click "OK" to close the Preferences window.
	- Go to Tools > Board > Boards Manager....
	- In the Boards Manager, type "ESP8266" into the search bar. 
			- You will see an entry for "ESP8266 by ESP8266 Community."
			- Click on it and then click the "Install" button to install the latest 
         version of the ESP8266 core for Arduino.
	- After the installation is complete, go to Tools > Board and scroll down 
    to the "ESP8266 Boards" section.
			- Select "Generic ESP8266 Board."


You will need to install these libraries for get_weather to work:
1. ESP8266 Board Support Package
Before you can use any ESP8266-specific libraries, you must first install the board support package for the ESP8266 in your Arduino IDE. This package provides the core libraries needed for network connectivity.

2. ESP8266WiFi Library
This library is a core part of the ESP8266 board package and is essential for connecting your board to a Wi-Fi network. You will use it to configure your Wi-Fi credentials and establish a connection to the internet.

3. ESP8266HTTPClient Library
Also included with the ESP8266 board package, this library provides the functionality to make HTTP and HTTPS requests. It's the equivalent of libcurl in the Arduino ecosystem, allowing you to send a request to the Open-Meteo API URL.

4. ArduinoJson Library
This is the standard and highly recommended library for parsing JSON data on the Arduino platform. It is lightweight, memory-efficient, and specifically designed for use on microcontrollers. You would install this library using the Arduino IDE's built-in Library Manager.


And lastly, 
1. You will need to enter your WiFi SSID and Password in the code
2. You will need to change the latitude and longitude in the weather URL to your current location (and perhaps change it away from the JP model. See Open-Meteo.com for more info.)
3. Rename this file as current_weather.ino
4. Open current_weather.ino in Arduino IDE
5. Send to your ESP8266 board with HW-364a display
