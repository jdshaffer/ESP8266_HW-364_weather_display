//------------------------------------------------------------------------------------
// "Current Weather with Button" for HW-364a and HW-364b (ESP8266 with OLED Display)
// Jeffrey D. Shaffer and Google Gemini
// 2025-08-07
//
//------------------------------------------------------------------------------------
// Notes:
//    - Defaults to the author's current location in Shizuoka, Japan
//    - Uses Celsius instead of Fahrenheit (Sorry!) 
//    - You can press the "Flash" button to toggle between normal-size
//      text and large-size text (2x larger) 
//
// Be Sure To:
//    - Be sure to add all the necessary libraries to Arduino IDE
//      (Check the libraries used in the #include section)
//    - Be sure to use your own ssid and network password in the 
//      "Wi-Fi Configuration" section below
//    - Be sure to change the latitude and longitude for your location
//      in the "Weather API Configuration" section belo
//
//------------------------------------------------------------------------------------

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

// OLED Display Configuration
#define SCREEN_WIDTH 128      // OLED display width, in pixels
#define SCREEN_HEIGHT 64      // OLED display height, in pixels
#define OLED_RESET -1         // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // The I2C address of the display
#define OLED_SDA 14           // Correct SDA pin for your wiring (D6 on most boards)
#define OLED_SCL 12           // Correct SCL pin for your wiring (D5 on most boards)
#define REFRESH_INTERVAL 15   // How often (in minutes) to refresh the data

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button-press Configuration
const int buttonPin = 0; // GPIO0
unsigned long lastPressTime = 0;
const int debounceDelay = 50; // Milliseconds
int text_size = 1;   // Start at text size 1 (default)

// Wi-Fi Configuration
const char* ssid = "Add Your WiFi SSID Here";
const char* password = "Add Your Password Here";

// Weather API Configuration
const char* server_host = "api.open-meteo.com";
const char* server_path = "/v1/forecast?latitude=34.9717465&longitude=138.378599&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m&timezone=Asia%2FTokyo&models=jma_seamless";

// Variables to store WX data
// (Global on purpose, so display_weather() can access it every time the button is pressed)
double temp_c;
double feels_like_c;
double humidity_percent;
double pressure_hpa;
double wind_speed_kph;
double wind_direction_deg;
double cloud_cover_percent;
double precipitation_mm;
String formattedTime;


// Variables for the timer
unsigned long previousMillis = 0;
const long interval = REFRESH_INTERVAL * 60 * 1000; // 15 minutes in milliseconds


// NTPClient configuration
// The second argument is for the timezone offset in seconds.
// Japan Standard Time (JST) is UTC+9, so 9 * 3600 = 32400 seconds.
WiFiUDP ntpUDP;
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


// Function to display the weather data
void display_weather(){
    if (text_size == 1) {   // If the text should be normal size
        // Clear the screen and prepare to write
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(text_size);
        display.setTextColor(SSD1306_WHITE);

        display.printf("Updated: %s\n", formattedTime);
        display.printf("Temp:    %.1f c\n", temp_c);
        display.printf("Feels:   %.1f c\n", feels_like_c);
        display.printf("Hum:     %.1f%%\n", humidity_percent);
        display.printf("Press:   %.1f hPa\n", pressure_hpa);
        display.printf("Wind:    %.1f mps %s\n", wind_speed_kph/3.6, convert_wind_to_compass(wind_direction_deg));   // Convert kph to mps inline
        display.printf("Cloud:   %.1f%%\n", cloud_cover_percent);
        display.printf("Precip:  %.1f mm\n", precipitation_mm);

        // Update the screen with current data
        display.display();

    } else {   // If the text should be double size
        // Clear the screen and prepare to write
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(text_size);
        display.setTextColor(SSD1306_WHITE);
        
        display.printf("Time %s\n", formattedTime);
        display.printf("Temp %.1f\n", temp_c);
        display.printf("Feel %.1f\n", feels_like_c);
        display.printf("Hum  %.0f%%\n", humidity_percent);
        
        // Update the screen with current data
        display.display();
    }
}


// Function to fetch the weather data
void fetch_weather(){
    // Before fetching the weather, update the time
    timeClient.update();
    
    // Get hours and minutes separately
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();

    // Reset the time string with each fetch
    formattedTime = "";
    // Create a string in HH:MM format, including leading zeros if needed
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
    display.println("Fetching WX data...");
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
                temp_c = current["temperature_2m"];
                feels_like_c = current["apparent_temperature"];
                humidity_percent = current["relative_humidity_2m"];
                pressure_hpa = current["surface_pressure"];
                wind_speed_kph = current["wind_speed_10m"];
                wind_direction_deg = current["wind_direction_10m"];
                cloud_cover_percent = current["cloud_cover"];
                precipitation_mm = current["precipitation"];

                // Call the display_weather function
                display_weather();
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
    
    // Configure the GPIO pin (Flash button) as an input
    pinMode(buttonPin, INPUT_PULLUP);

    // Initialize I2C communication on the correct pins
    Wire.begin(OLED_SDA, OLED_SCL);

    // Initialize the OLED display with the correct address
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }

    display.clearDisplay();
    display.setTextSize(text_size);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println("Connecting to...");
    display.println();
    display.println(ssid);
    display.display();

    // Try conneting to network with configured ssid and password
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    // Update display with Wi-Fi status
    display.clearDisplay();
    display.setCursor(0,0);
    display.println("WiFi is up!");
    display.println();
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
    delay(2000);

    // Initial weather fetch and display at startup
    fetch_weather();
    display_weather();

    // Store the time of the first fetch
    previousMillis = millis();
}

void loop() {
    unsigned long currentMillis = millis();

    // Check if the update interval has passed
    if (currentMillis - previousMillis >= interval) {
        // Save the current time for the next interval
        previousMillis = currentMillis;

        // Re-run the weather fetching function
        fetch_weather();
        display_weather();
        // fetchAndDisplayWeather();
    }

    // Read button state
    int buttonState = digitalRead(buttonPin);
    unsigned long currentTime = millis();

    // Check for a button press (LOW state) and debounce it
    if (buttonState == LOW && (currentTime - lastPressTime) > debounceDelay) {
        // The button has been pressed and it's not a bounce
        lastPressTime = currentTime;

        // Toggle the text size state
        if (text_size == 1) {   // If currently size 1, set to 2
            text_size = 2;
        } else {                // If not currently size 1, set to 1
            text_size = 1;
        }

        // Display the weather data with new text size
        display_weather();
    }
}
