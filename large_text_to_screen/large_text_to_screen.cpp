//------------------------------------------------------------------------------
// "Large Text to Screen"
// Simple code example for HW-364a and HW-364b development boards
// Jeffrey D. Shaffer
// 2025-08-07
//
//------------------------------------------------------------------------------
// Notes:
//    - This is the most minimal program I would write to demonstrate writing
//      larger text to the OLED screen
//    - This is essentially the same as "Text to Screen", but the
//      display.setTextSize is set to 2 (double) instead of 1 (norma)
//    - Only the Adafruit_SSD1306 library is necessary for this to work
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


void write_to_display(){
    // We start by clearing the display each time
    display.clearDisplay();
    // We set the text size (1=normal, 2=double, etc.)
    display.setTextSize(2);
    // Start writing at the top-left
    display.setCursor(0,0);
    // We really can't change the colors of the built-in display,
    // but the program expects a color to be set regardless
    display.setTextColor(SSD1306_WHITE);
    // Write text to the screen (we can use println, printf, etc.)
    // Just remember is you use printf, you'll need to add a newline!
    display.println("Top line  ");
    display.println("is orange.");
    display.printf ("The others\n");
    display.printf ("are blue. \n");

    // You must call this to update the screen with our data
    display.display();
}


void setup() {
    // Initialize I2C communication on the correct pins as per your working sketch
    Wire.begin(OLED_SDA, OLED_SCL);

    // Initialize the OLED display with the correct address
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}


void loop() {
    write_to_display();
}
