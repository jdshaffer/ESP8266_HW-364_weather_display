//------------------------------------------------------------------------------------
// Weather Display (v1.4a)
// for HW-364a and HW-364b development boards
// Jeffrey D. Shaffer
// 2025-08-17
//
//------------------------------------------------------------------------------------
// This program fetches the current weather conditions every 30 minutes and
// displays it on the built-in OLED display. To save energy, it puts the WiFi
// to sleep between data fetches.
//
// After a network connection problem, it displays a notification, waits a while,
//then tries to reconnect.
//
//------------------------------------------------------------------------------------
// Notes:
//    - Defaults to Suruga-ku, Shizuoka, Japan
//    - Many settings are configurable
//    - Don't forget to use your SSID and Wi-Fi password
//
//------------------------------------------------------------------------------------
// Warnings:
//    - Power draw is so small, USB-batteries may automatically power down
//    - Running from 2 AAAs might require a capacitor soldered inline between the
//      battery pack and the board. (I ccouldn't tell if my problems were wifi-related
//      or power-related, so I added a 25V 240uF capacitor, but anything over 10W
//      would also probably work).
//    - Google Gemini estimates 390 hours of use when running on two AAAs.
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
#define SCREEN_WIDTH 128          // OLED display width, in pixels
#define SCREEN_HEIGHT 64          // OLED display height, in pixels
#define OLED_RESET -1             // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C       // The I2C address of the display
#define OLED_SDA 14               // Correct SDA pin for your wiring (D6 on most boards)
#define OLED_SCL 12               // Correct SCL pin for your wiring (D5 on most boards)
#define REFRESH_INTERVAL 30       // How often (in minutes) to refresh the data

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button-press Configuration
const int buttonPin = 0;          // Use the "Flash" butoon (GPIO0)
unsigned long lastPressTime = 0;  // Used for timing the debounce delay
const int debounceDelay = 200;    // Debounce delay duration in milliseconds
int user_selected_text_size = 1;  // Start at text size 1 (default)

// Wi-Fi Configuration
const char* ssid = "YOUR SSID GOES HERE";
const char* password = "YOUR WIFI PASSWORD GOES HERE";
int maxAttempts = 3;   // Max number of wi-fi connection attempts to try

// Weather API Configuration
const char* server_host = "api.open-meteo.com";
const char* server_path = "/v1/forecast?latitude=34.9717465&longitude=138.378599&current=temperature_2m,relative_humidity_2m,apparent_temperature,is_day,precipitation,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m&timezone=Asia%2FTokyo&models=jma_seamless";

// Variables for Storing WX Data
// Global on purpose, so display_weather() can access it every time the button is pressed
double temp_c;
double feels_like_c;
double humidity_percent;
double pressure_hpa;
double wind_speed_kph;
double wind_direction_deg;
double cloud_cover_percent;
double precipitation_mm;
String formattedTime;

// Variables for the Timer
unsigned long previousMillis = 0;
long interval = REFRESH_INTERVAL * 60 * 1000;

// NTPClient Configuration
// The second argument is for the timezone offset in seconds.
// Japan Standard Time (JST) is UTC+9, so 9 * 3600 = 32400 seconds.
WiFiUDP ntpUDP;
const long utcOffsetInSeconds = 9 * 3600;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// Function to display single-line messages
void display_message(const char* MESSAGE, const int MESSAGE_TEXT_SIZE, const int MESSAGE_DURATION){
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(MESSAGE_TEXT_SIZE);
    display.setTextColor(SSD1306_WHITE);
    display.printf("%s", MESSAGE);
    display.display();
    delay(MESSAGE_DURATION * 1000);   // Convert input seconds to milliseconds
}


// Function to Display the Pre-fetched Weather Data
void display_weather(){
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(user_selected_text_size);
    display.setTextColor(SSD1306_WHITE);

    if (user_selected_text_size == 1) {   // If the text should be normal size
        display.printf("  Temp    %6.1f C\n", temp_c);
        display.printf("  Feels   %6.1f C\n", feels_like_c);
        display.printf("  Hum     %6.1f %%\n", humidity_percent);
        display.printf("  Press   %4.1f hPa\n", pressure_hpa);
        display.printf("  Wind    %6.1f mps\n", wind_speed_kph/3.6);   // Convert kph to mps inline
        display.printf("  Cloud   %6.1f %%\n", cloud_cover_percent);
        display.println();
        display.printf("   (Updated %s)\n", formattedTime.c_str());
        display.display();

    } else {   // If the text should be double size
        display.printf("Temp  %2.1f\n", temp_c);
        display.printf("Feel  %2.1f\n", feels_like_c);
        display.printf("Hum   %2.0f %%\n", humidity_percent);
        display.printf("  (%s) \n", formattedTime.c_str());
        display.display();
    }
}


// Function to Connect to Wi-Fi
bool connect_to_wifi() {
    // Wake up Wi-Fi and wait for it to turn on
    WiFi.forceSleepWake();
    delay(50);

    // Completely turn off the Wi-Fi before trying to reconnect
    WiFi.mode(WIFI_OFF);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);    // Set the Wi-Fi mode back to station mode

    // Attempt to connect to wi-fi  (maxAttempts configured at top of program)
    for (int attempt = 1; attempt <= maxAttempts; attempt++) {
        WiFi.begin(ssid, password);
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(1);
        display.printf(" Connecting to WiFi \n");
        display.printf("   Attempt %d of %d\n", attempt, maxAttempts);
        display.display();

        // Wait for up to 10 seconds (10000 milliseconds) for a connection
        long start_time = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - start_time) < 10000) {
            delay(500);
            display.printf(".");
            display.display();
        }

        if (WiFi.status() == WL_CONNECTED) {
            delay(500);      // Give the network stack a little time to finish connecting
            return true;     // Exit the "attempt connection" loop if connected
        }
    }

    // If it refuses to connect, grab the error info and put the WiFi back to sleep
    int status = WiFi.status();
    WiFi.forceSleepBegin(); // Put Wi-Fi back to sleep before halting

    // Tell the user we couldn't connect and display error message
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println(" Failed to connect  ");
    display.printf ("   after %d tries   \n", maxAttempts);
    display.println("--------------------");
    display.println("WiFi Status Report:");
    display.println();
    switch (status) {   // Display what the connection error code was
        case WL_NO_SSID_AVAIL:
            display.println("Network not found");
            display.println("Check SSID name");
            display.display();
            break;   // Break out of the switch case, and cause the program to halt
        case WL_WRONG_PASSWORD:
            display.println("Wrong password");
            display.println("Check password");
            display.display();
            break;   // Break out of the switch case, and cause the program to halt
        case WL_DISCONNECTED:     // Fall through to the next case
        case WL_CONNECT_FAILED:   // Fall through to the next case
        case WL_CONNECTION_LOST:  // Display a wait message for 5 minutes, then retry connecting
            // This is a temporary error; we'll retry later.
            display_message("    Disconnected\n    from network\n\n Waiting 5 minutes\n  before retrying\n", 1, 5 * 60);
            break;
        default:
            display.printf("Status code: %d\n", status);
            display.println("Unknown error");
            display.display();
            break;   // Break out of the switch case, and cause the program to halt
    }
    return false;
}


// Function to Fetch and Display the Weather Data
void fetch_and_display_weather() {
    // Fetch the time
    timeClient.update();

    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();

    formattedTime = "";
    if (currentHour < 10) formattedTime += "0";
    formattedTime += String(currentHour);
    formattedTime += ":";
    if (currentMinute < 10) formattedTime += "0";
    formattedTime += String(currentMinute);

    WiFiClientSecure client;
    client.setInsecure(); // Accept all certificates for convenience
    HTTPClient http;

    display_message(" Fetching WX Data...", user_selected_text_size, 1);

    if (http.begin(client, server_host, 443, server_path)) {
        int httpCode = http.GET();
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                String payload = http.getString();
                StaticJsonDocument<1024> doc;
                DeserializationError error = deserializeJson(doc, payload);

                if (!error) {
                    JsonObject current = doc["current"];
                    temp_c = current["temperature_2m"];
                    feels_like_c = current["apparent_temperature"];
                    humidity_percent = current["relative_humidity_2m"];
                    pressure_hpa = current["surface_pressure"];
                    wind_speed_kph = current["wind_speed_10m"];
                    wind_direction_deg = current["wind_direction_10m"];
                    cloud_cover_percent = current["cloud_cover"];
                    precipitation_mm = current["precipitation"];
                    display_weather();
                } else {
                    display_message("JSON Error!\n", 1, 3);
                }
            } else {
                display_message("HTTP Error!\n", 1, 3);
            }
        } else {
            display_message("Connection error!\n", 1, 3);
        }
        http.end();
    }
}


void setup() {
    // Configure the GPIO pin (Flash button) as an input
    pinMode(buttonPin, INPUT_PULLUP);

    // Initialize I2C (display) communication on the correct pins
    Wire.begin(OLED_SDA, OLED_SCL);

    // Initialize the OLED display with the correct address
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        // A failed display is a fatal error for this device, so we halt.
        // for(;;);
    }

    // Print a boot message (mostly to clear the screen)
    display_message("     WX Display\n       by Jds", 1, 2);
    // Run the initial weather fetching function
    if (connect_to_wifi()) {
        fetch_and_display_weather();
        WiFi.forceSleepBegin(); // Put the Wi-Fi module back to sleep
    } else {
        // If a SSID or password error occurs, halt the program
        int status = WiFi.status();
        if (status == WL_NO_SSID_AVAIL || status == WL_WRONG_PASSWORD) {
            while(true) {
                delay(1000);
            }
        }
    }

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
        if (connect_to_wifi()) {
            fetch_and_display_weather();
            WiFi.forceSleepBegin(); // Put the Wi-Fi module back to sleep
        } else {
             // If a SSID or password error occurs, halt the program
             int status = WiFi.status();
             if (status == WL_NO_SSID_AVAIL || status == WL_WRONG_PASSWORD) {
                 while(true) {
                     delay(1000);
                 }
             }
        }
    }

    // Read button state
    int buttonState = digitalRead(buttonPin);
    unsigned long currentTime = millis();

    // Check for a button press (LOW state) and debounce it
    if (buttonState == LOW && (currentTime - lastPressTime) > debounceDelay) {
        // The button has been pressed and it's not a bounce
        lastPressTime = currentTime;

        // Toggle the text size state
        if (user_selected_text_size == 1) {   // If currently size 1, set to 2
            user_selected_text_size = 2;
        } else {                // If not currently size 1, set to 1
            user_selected_text_size = 1;
        }

        // Display the weather data with new text size
        display_weather();
    }
}
