//------------------------------------------------------------------------------
// Bouncing Ball
// Adafruit_GFX Test for HW-364a and HW-364b development boards
// Jeffrey D. Shaffer
// 2025-08-07
//
// Notes:
//    - Color doesn't matter with the built-in OLED display
//    - Top 16 rows are always orange
//    - Bottom 48 rows are always blue
//    - Width = 128
// 
//------------------------------------------------------------------------------

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// OLED Display Configuration
#define SCREEN_WIDTH 128      // OLED display width,  in pixels
#define SCREEN_HEIGHT 64      // OLED display height, in pixels
#define OLED_RESET -1         // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C   // The I2C address of the display
#define OLED_SDA 14           // Correct SDA pin for your wiring (D6 on most boards)
#define OLED_SCL 12           // Correct SCL pin for your wiring (D5 on most boards)

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Setup for ball motion
int x_pos =  4;          // Runs from 0 to 128 (minus half the ball_radius)
int y_pos = 20;          // Runs from 8 to  64 (minus half the ball_radius)  
int ball_radius = 4;     // Size of the ball
int x_motion = 2;        // Motion in the x direction
int y_motion = 3;        // Motion in the y direction


// Function to update the position of the ball with each loop
void update_ball_position() {
    // Update position
    x_pos += x_motion;
    y_pos += y_motion;

    // Check left-right boundaries
    if ((x_pos - ball_radius) <= 0 || (x_pos + ball_radius) >= 128) {
        x_motion *= -1;  // Reverse x direction
    }

    // Check top-bottom boundaries   (avoiding top 16 rows of orange pixels)
    if ((y_pos - ball_radius) <= 16 || (y_pos + ball_radius + (ball_radius/2)) >= 64) {
        y_motion *= -1;  // Reverse y direction
    }
}


// Function to draw the text at the top (in the orange area) and the ball (in the blue area)
void draw_to_the_screen(){
    display.clearDisplay();

    // Display position information in the top orange area
    display.setTextSize(1);
    display.setCursor(0,0);
    display.setTextColor(SSD1306_WHITE);       // It'll be orange no matter what you put here
    display.printf ("X = %3d      Y = %3d\n", x_pos, y_pos);   // Write the pos into to screen
    display.println("____________________");   // Just makes it look nice, like a top wall

    // Draw the ball (a filled circle) -- color will be blue no matter what
    display.fillCircle(x_pos, y_pos, ball_radius, WHITE);

    display.display();
}


void setup() {
    Wire.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
}


void loop() {
    update_ball_position();
    draw_to_the_screen();
    delay(20);   // Add a small delay to make the movement visible
}
