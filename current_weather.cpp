//------------------------------------------------------------
// "Current Weather" for ESP8266 with HW-364a OLED Display
// Jeffrey D. Shaffer and Google Gemini
// 2025-08-06
//
//------------------------------------------------------------
// Notes:
//    - Current version displays text at 2x for readability
//         (Set display.setTextSize to 2)
//
//    - Current program displays weather information for
//      Shizuoka, Japan (where I live)
//
//    - "Time" in the display is the time when the data
//       was last updated.
//
//------------------------------------------------------------
// How to Make it Work:
//    - You will need to enter your WiFi SSID and Password
//      in the code below
//    - You will need to change the latitude and longitude
//      in the weather URL to your current location (and 
//      perhaps change it away from the JP model. See 
//      Open-Meteo.com for more info.)
//    - Rename this file as current_weather.ino
//    - Open current_weather.ino in Arduino IDE
//    - Install these libraries into Arduino IDE:
//         - ArduinoJson
//         - Adafruit GFX Library
//         - Adafruit SSD1306
//    - Send to your ESP8266 board with HW-364a display
//
//------------------------------------------------------------

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

// Required for the OLED display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Required for getting the time from the internet
#include <NTPClient.h>
#include <WiFiUdp.h>

// Wi-Fi network credentials
const char* ssid = "Your SSID Goes Here";
const char* password = "Your WiFi Password Goes Here";


// Open-Meteo API Configuration
const char* server_host = "api.open-meteo.com";
const char* server_path = "/v1/forecast?latitude=34.9717465&longitude=138.378599&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m&timezone=Asia%2FTokyo&models=jma_seamless";

// OLED Display Configuration
#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 64      // OLED display height, in pixels
#define OLED_RESET -1         // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // The I2C address of the display
#define OLED_SDA 14           // Correct SDA pin for your wiring (D6 on most boards)
#define OLED_SCL 12           // Correct SCL pin for your wiring (D5 on most boards)
#define REFRESH_INTERVAL 15   // How often (in minutes) to refresh the data

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Variables for the timer
unsigned long previousMillis = 0;
const long interval = REFRESH_INTERVAL * 60 * 1000; // 15 minutes in milliseconds

// NTPClient configuration
WiFiUDP ntpUDP;
// The second argument is for the timezone offset in seconds.
// Japan Standard Time (JST) is UTC+9, so 9 * 3600 = 32400 seconds.
const long utcOffsetInSeconds = 9 * 3600;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

// Function to convert wind direction (degrees) to compass directions
const char* convert_wind_to_compass(double wind_dir) {
    if (wind_dir >= 0 && wind_dir < 22.5) return "N";
    if (wind_dir >= 22.5 && wind_dir < 45) return "NNE";
    if (wind_dir >= 45 && wind_dir < 67.5) return "NE";
    if (wind_dir >= 67.5 && wind_dir < 90) return "ENE";
    if (wind_dir >= 90 && wind_dir < 112.5) return "E";
    if (wind_dir >= 112.5 && wind_dir < 135) return "ESE";
    if (wind_dir >= 135 && wind_dir < 157.5) return "SE";
    if (wind_dir >= 157.5 && wind_dir < 180) return "SSE";
    if (wind_dir >= 180 && wind_dir < 202.5) return "S";
    if (wind_dir >= 202.5 && wind_dir < 225) return "SSW";
    if (wind_dir >= 225 && wind_dir < 247.5) return "SW";
    if (wind_dir >= 247.5 && wind_dir < 270) return "WSW";
    if (wind_dir >= 270 && wind_dir < 292.5) return "W";
    if (wind_dir >= 292.5 && wind_dir < 315) return "WNW";
    if (wind_dir >= 315 && wind_dir < 337.5) return "NW";
    if (wind_dir >= 337.5 && wind_dir < 360) return "NNW";
    if (wind_dir == 360) return "N";
    return "Unknown";
}

// New function to fetch and display weather data
void fetchAndDisplayWeather() {
    // Before fetching the weather, update the time
    timeClient.update();
    
    // Get hours and minutes separately
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();

    // Create a string in HH:MM format, including leading zeros if needed
    String formattedTime = "";
    if (currentHour < 10) {
        formattedTime += "0";
    }
    formattedTime += String(currentHour);
    formattedTime += ":";
    if (currentMinute < 10) {
        formattedTime += "0";
    }
    formattedTime += String(currentMinute);

    // Use WiFiClientSecure for HTTPS requests
    WiFiClientSecure client;
    client.setInsecure(); // Accept all certificates (for convenience)
    HTTPClient http;

    display.clearDisplay();
    display.setCursor(0,0);
    display.println("Fetching");
    display.println("WX data...");
    display.display();

    // Begin the HTTP request
    if (http.begin(client, server_host, 443, server_path)) {
        int httpCode = http.GET();

        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                // Get the JSON response string
                String payload = http.getString();

                // StaticJsonDocument is memory efficient for microcontrollers
                StaticJsonDocument<1024> doc; // Adjust size as needed
                DeserializationError error = deserializeJson(doc, payload);

                if (error) {
                    display.clearDisplay();
                    display.setCursor(0,0);
                    display.println("JSON Error!");
                    display.println(error.c_str());
                    display.display();
                    return;
                }

                JsonObject current = doc["current"];
                double temp_c = current["temperature_2m"];
                double feels_like_c = current["apparent_temperature"];
                double humidity_percent = current["relative_humidity_2m"];
                double pressure_hpa = current["surface_pressure"];
                double wind_speed_kph = current["wind_speed_10m"];
                double wind_direction_deg = current["wind_direction_10m"];
                double cloud_cover_percent = current["cloud_cover"];
                double precipitation_mm = current["precipitation"];

                // Clear the display and start printing weather info
                display.clearDisplay();
                display.setCursor(0,0);
                display.setTextSize(2);
                display.setTextColor(SSD1306_WHITE);
                
                display.println("Time " + formattedTime);
                display.printf("Temp: %.1f\n", temp_c);
                display.printf("Feel: %.1f\n", feels_like_c);
                display.printf("Hum : %.0f%%\n", humidity_percent);
                
                // You must call this to actually update the screen!
                display.display();
            }
        } else {
            display.clearDisplay();
            display.setCursor(0,0);
            display.println("HTTP Error!");
            display.println(http.errorToString(httpCode).c_str());
            display.display();
        }

        http.end(); // Free resources
    } else {
        display.clearDisplay();
        display.setCursor(0,0);
        display.println("Connection");
        display.println("error!");
        display.display();
    }
}


void setup() {
    Serial.begin(115200);
    delay(10);
    
    // Initialize I2C communication on the correct pins as per your working sketch
    Wire.begin(OLED_SDA, OLED_SCL);

    // Initialize the OLED display with the correct address
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("Connecting");
    display.println("to network");
    display.println(ssid);
    display.display();

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Update display with Wi-Fi status
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi up!");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);

    // Initial weather fetch at startup
    fetchAndDisplayWeather();

    // Store the time of the first fetch
    previousMillis = millis();
}

void loop() {
    unsigned long currentMillis = millis();

    // Check if the interval has passed
    if (currentMillis - previousMillis >= interval) {
        // Save the current time for the next interval
        previousMillis = currentMillis;

        // Re-run the weather fetching function
        fetchAndDisplayWeather();
    }
}