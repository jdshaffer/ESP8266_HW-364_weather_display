//------------------------------------------------------------------------------
// "Button Press Example"
// Simple code example for HW-364a and HW-364b development boards
// Demonstrates how to change text-size using a button press
// Jeffrey D. Shaffer
// 2025-08-07
//
//------------------------------------------------------------------------------
// Notes:
//    - Uses a "debouncing" technique that introduces a short delay before 
//      re-reading the button to prevent the program from registering 
//      multiple button presses from a single physical press.
//    - The 150 ms debounceDelay is about right for my slow fingers. You
//      can "test" this speed by holding down the "Flash" button when your
//      program is running.
//
//------------------------------------------------------------------------------

// Required for the OLED display
#include <Adafruit_SSD1306.h>


// OLED Display Configuration
#define SCREEN_WIDTH 128      // OLED display width,  in pixels
#define SCREEN_HEIGHT 64      // OLED display height, in pixels
#define OLED_RESET -1         // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // The I2C address of the display
#define OLED_SDA 14           // Correct SDA pin for your wiring (D6 on most boards)
#define OLED_SCL 12           // Correct SCL pin for your wiring (D5 on most boards)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Configuration for the button-press
const int buttonPin = 0;          // We're going to use GPIO0 (Flash) for our button
bool largeText = false;           // A variable used to track the current text size
unsigned long lastPressTime = 0;  // Used to help debounce button presses 
const int debounceDelay = 150;    // Set the debounce delay in milliseconds.


void setup() {
    // Initialize I2C communication on the correct pins to talk to our display
    Wire.begin(OLED_SDA, OLED_SCL);

    // Initialize the OLED display and clear it
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    // Setup the "Flash" button to be used
    pinMode(buttonPin, INPUT_PULLUP);
}


void loop() {
    // Read in the button state (pressed or not) and save as "buttonState"
    int buttonState = digitalRead(buttonPin);
    // Start our little timer (to help stablize the button reads)
    unsigned long currentTime = millis();

    // Check for a button press (LOW state) and debounce it
    if (buttonState == LOW && (currentTime - lastPressTime) > debounceDelay) {
    // The button has been pressed and it's not a bounce
    lastPressTime = currentTime;

    // Toggle the text size state
    largeText = !largeText;

    // Clear the display and draw the text with the new size
    display.clearDisplay();
    display.setCursor(0, 0);

    if (largeText) {
        display.setTextSize(2);
        display.println("Large Text");
    } else {
        display.setTextSize(1);
        display.println("Small Text");
    }

    display.display();
    }

}



