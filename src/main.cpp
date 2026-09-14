// Two Button Click Demo for ESP32-2432S028R
// Displays two buttons and shows a message when clicked
// Based on TouchTest.cpp

#include "Arduino.h"
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

// TFT Display Pins (also set via platformio.ini build_flags for TFT_eSPI)
#define TFT_BL 21

// Touch Screen Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Calibration values for XPT2046 (raw ADC to screen coordinates)
// Verified working values from serial log - see HARDWARE_NOTES.md
uint16_t ts_minx = 489;
uint16_t ts_miny = 283;
uint16_t ts_maxx = 3772;
uint16_t ts_maxy = 3713;

// Set to 1 to run the interactive 4-point calibration routine at boot instead
// of using the hardcoded values above (e.g. if you swap to a different unit)
#define RUN_TOUCH_CALIBRATION 0

// Display dimensions - physical panel renders portrait at rotation(0)
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

// Button positions - stacked vertically for portrait, shared by calibration and setup
const uint16_t BTN1_X = 20, BTN1_Y = 30, BTN1_W = 200, BTN1_H = 100;
const uint16_t BTN2_X = 20, BTN2_Y = 160, BTN2_W = 200, BTN2_H = 100;

// Button structure
struct Button
{
    uint16_t x, y, width, height;
    uint16_t color;
    char label[20];
};

// Initialize SPI instance for touch (display uses its own bus via TFT_eSPI)
SPIClass vspi = SPIClass(VSPI);

// Initialize touch and display
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

// Button instances
Button button1, button2;

// Function to calibrate touch coordinates to screen coordinates
void calibrateTouch(TS_Point &p)
{
    // Map raw ADC values to screen coordinates
    // In portrait mode: X should map to 0-240, Y should map to 0-320
    uint16_t calibrated_x = map(p.x, ts_minx, ts_maxx, 0, SCREEN_WIDTH);
    uint16_t calibrated_y = map(p.y, ts_miny, ts_maxy, 0, SCREEN_HEIGHT);

    p.x = calibrated_x;
    p.y = calibrated_y;
}

// Function to draw a button
void drawButton(Button &btn)
{
    tft.fillRect(btn.x, btn.y, btn.width, btn.height, btn.color);
    tft.drawRect(btn.x, btn.y, btn.width, btn.height, TFT_BLACK);

    // Draw button text, centered
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLACK, btn.color);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(btn.label, btn.x + btn.width / 2, btn.y + btn.height / 2);
    tft.setTextDatum(TL_DATUM);
}

// Function to check if a point is within a button
boolean isPointInButton(TS_Point p, Button &btn)
{
    return (p.x >= btn.x && p.x <= btn.x + btn.width &&
            p.y >= btn.y && p.y <= btn.y + btn.height);
}

// Function to display a message on screen
void displayMessage(const char *msg, uint16_t color)
{
    // Clear the message area at bottom of the screen
    tft.fillRect(0, SCREEN_HEIGHT - 30, SCREEN_WIDTH, 30, TFT_BLACK);

    // Display the message
    tft.setTextSize(1);
    tft.setTextColor(color, TFT_BLACK);
    tft.setCursor(5, SCREEN_HEIGHT - 20);
    tft.print(msg);
}

// Calibration routine - 4-point calibration: corners + button centers
void calibrationRoutine()
{
    uint16_t cal_minx = 4095, cal_miny = 4095;
    uint16_t cal_maxx = 0, cal_maxy = 0;
    TS_Point p1_center, p2_center;

    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(10, 10);
    tft.print("4-POINT CALIBRATION");
    tft.setCursor(10, 30);
    tft.print("Display: 240w x 320h");
    delay(1500);

    // Point 1: Top-left corner
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5, 50);
    tft.print("Touch");
    tft.setCursor(5, 80);
    tft.print("TOP-LEFT");
    tft.setTextSize(1);
    tft.setCursor(5, 120);
    tft.print("Point 1 of 4");
    tft.drawRect(5, 5, 20, 20, TFT_YELLOW);

    boolean touched = false;
    while (!touched)
    {
        if (ts.touched())
        {
            TS_Point p = ts.getPoint();
            cal_minx = min(cal_minx, (uint16_t)p.x);
            cal_miny = min(cal_miny, (uint16_t)p.y);
            Serial.print("[CAL P1] TL: X=");
            Serial.print(p.x);
            Serial.print(", Y=");
            Serial.println(p.y);
            touched = true;
            delay(500);
        }
    }

    // Point 2: Bottom-right corner
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5, 50);
    tft.print("Touch");
    tft.setCursor(5, 80);
    tft.print("BOTTOM-RIGHT");
    tft.setTextSize(1);
    tft.setCursor(5, 120);
    tft.print("Point 2 of 4");
    tft.drawRect(SCREEN_WIDTH - 25, SCREEN_HEIGHT - 25, 20, 20, TFT_YELLOW);

    touched = false;
    while (!touched)
    {
        if (ts.touched())
        {
            TS_Point p = ts.getPoint();
            cal_maxx = max(cal_maxx, (uint16_t)p.x);
            cal_maxy = max(cal_maxy, (uint16_t)p.y);
            Serial.print("[CAL P2] BR: X=");
            Serial.print(p.x);
            Serial.print(", Y=");
            Serial.println(p.y);
            touched = true;
            delay(500);
        }
    }

    // Point 3: Button 1 center (should be near top center)
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5, 50);
    tft.print("Touch");
    tft.setCursor(5, 80);
    tft.print("Button 1");
    tft.setTextSize(1);
    tft.setCursor(5, 120);
    tft.print("Point 3 of 4");
    tft.drawRect(BTN1_X + BTN1_W / 2 - 15, BTN1_Y + BTN1_H / 2 - 15, 30, 30, TFT_GREEN);

    touched = false;
    while (!touched)
    {
        if (ts.touched())
        {
            p1_center = ts.getPoint();
            Serial.print("[CAL P3] B1: X=");
            Serial.print(p1_center.x);
            Serial.print(", Y=");
            Serial.println(p1_center.y);
            touched = true;
            delay(500);
        }
    }

    // Point 4: Button 2 center (should be near bottom center)
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5, 50);
    tft.print("Touch");
    tft.setCursor(5, 80);
    tft.print("Button 2");
    tft.setTextSize(1);
    tft.setCursor(5, 120);
    tft.print("Point 4 of 4");
    tft.drawRect(BTN2_X + BTN2_W / 2 - 15, BTN2_Y + BTN2_H / 2 - 15, 30, 30, TFT_GREEN);

    touched = false;
    while (!touched)
    {
        if (ts.touched())
        {
            p2_center = ts.getPoint();
            Serial.print("[CAL P4] B2: X=");
            Serial.print(p2_center.x);
            Serial.print(", Y=");
            Serial.println(p2_center.y);
            touched = true;
            delay(500);
        }
    }

    // Store calibration values
    ts_minx = cal_minx;
    ts_miny = cal_miny;
    ts_maxx = cal_maxx;
    ts_maxy = cal_maxy;

    // Check if Y-axis is inverted (common with XPT2046)
    if (ts_miny > ts_maxy)
    {
        // Swap them
        uint16_t temp = ts_miny;
        ts_miny = ts_maxy;
        ts_maxy = temp;
        Serial.println("[INFO] Y-axis was inverted - swapped MINY/MAXY");
    }

    // Display calibration results
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(1);
    tft.setTextColor(TFT_GREEN);
    tft.setCursor(5, 10);
    tft.print("CALIBRATION COMPLETE");
    tft.setCursor(5, 30);
    tft.print("MinX:");
    tft.print(ts_minx);
    tft.print(" MaxX:");
    tft.println(ts_maxx);
    tft.setCursor(5, 50);
    tft.print("MinY:");
    tft.print(ts_miny);
    tft.print(" MaxY:");
    tft.println(ts_maxy);
    tft.setCursor(5, 70);
    tft.print("X range:");
    tft.println(ts_maxx - ts_minx);
    tft.setCursor(5, 90);
    tft.print("Y range:");
    tft.println(ts_maxy - ts_miny);
    tft.setCursor(5, 120);
    tft.print("B1 raw: X=");
    tft.print(p1_center.x);
    tft.print(" Y=");
    tft.println(p1_center.y);
    tft.setCursor(5, 140);
    tft.print("B2 raw: X=");
    tft.print(p2_center.x);
    tft.print(" Y=");
    tft.println(p2_center.y);

    Serial.println("\n=== CALIBRATION COMPLETE ===");
    Serial.print("TS_MINX = ");
    Serial.println(ts_minx);
    Serial.print("TS_MAXX = ");
    Serial.println(ts_maxx);
    Serial.print("TS_MINY = ");
    Serial.println(ts_miny);
    Serial.print("TS_MAXY = ");
    Serial.println(ts_maxy);
    Serial.print("X range: ");
    Serial.print(ts_maxx - ts_minx);
    Serial.print(", Y range: ");
    Serial.println(ts_maxy - ts_miny);
    Serial.print("Button 1 center (raw): X=");
    Serial.print(p1_center.x);
    Serial.print(", Y=");
    Serial.println(p1_center.y);
    Serial.print("Button 2 center (raw): X=");
    Serial.print(p2_center.x);
    Serial.print(", Y=");
    Serial.println(p2_center.y);

    delay(3000);
}

void setup()
{
    Serial.begin(38400);

    // Initialize backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // Initialize TFT display
    tft.init();
    tft.setRotation(0); // Physical panel renders landscape at rotation(0)
    tft.fillScreen(TFT_BLACK);

    // Wait for serial if needed
    while (!Serial && (millis() <= 1000))
        ;

    // DIAGNOSTIC: solid color fills only, no app logic, to isolate hardware vs software noise
    Serial.println("\n=== DIAGNOSTIC: solid fill test ===");
    Serial.print("tft.width()=");
    Serial.print(tft.width());
    Serial.print(" tft.height()=");
    Serial.println(tft.height());
    tft.fillScreen(TFT_RED);
    Serial.println("Filled RED - check for noise, then waiting 3s...");
    delay(3000);
    tft.fillScreen(TFT_GREEN);
    Serial.println("Filled GREEN - check for noise, then waiting 3s...");
    delay(3000);
    tft.fillScreen(TFT_BLACK);
    Serial.println("Filled BLACK - diagnostic done.");
    delay(1000);

    // Setup touch screen on VSPI
    vspi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(vspi);
    ts.setRotation(0); // Portrait mode to match display

    Serial.println("\nStarting Calibration Routine...");
    Serial.println("You will be asked to touch the top-left and bottom-right corners.");

    // Run calibration
#if RUN_TOUCH_CALIBRATION
    calibrationRoutine();
#else
    Serial.println("Skipping interactive calibration - using hardcoded values:");
    Serial.print("TS_MINX=");
    Serial.print(ts_minx);
    Serial.print(" TS_MAXX=");
    Serial.print(ts_maxx);
    Serial.print(" TS_MINY=");
    Serial.print(ts_miny);
    Serial.print(" TS_MAXY=");
    Serial.println(ts_maxy);
#endif

    // Initialize buttons - stacked vertically for portrait mode (240 wide x 320 tall)
    button1.x = BTN1_X;
    button1.y = BTN1_Y;
    button1.width = BTN1_W;
    button1.height = BTN1_H;
    button1.color = TFT_BLUE;
    strcpy(button1.label, "Button A");

    button2.x = BTN2_X;
    button2.y = BTN2_Y;
    button2.width = BTN2_W;
    button2.height = BTN2_H;
    button2.color = TFT_RED;
    strcpy(button2.label, "Button B");

    // Print button boundaries for debugging
    Serial.println("\n=== Button Boundaries ===");
    Serial.print("Button A: X=");
    Serial.print(button1.x);
    Serial.print("-");
    Serial.print(button1.x + button1.width);
    Serial.print(", Y=");
    Serial.print(button1.y);
    Serial.print("-");
    Serial.println(button1.y + button1.height);

    Serial.print("Button B: X=");
    Serial.print(button2.x);
    Serial.print("-");
    Serial.print(button2.x + button2.width);
    Serial.print(", Y=");
    Serial.print(button2.y);
    Serial.print("-");
    Serial.println(button2.y + button2.height);
    Serial.print("Screen dimensions: ");
    Serial.print(SCREEN_WIDTH);
    Serial.print("x");
    Serial.println(SCREEN_HEIGHT);
    tft.fillScreen(TFT_BLACK);

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(80, 10);
    tft.print("Touch Demo");

    drawButton(button1);
    drawButton(button2);

    displayMessage("Touch a button", TFT_GREEN);

    Serial.println("Setup complete. Touch a button!");
}

void loop()
{
    if (ts.touched())
    {
        // Get raw touch point
        TS_Point rawPoint = ts.getPoint();
        TS_Point p = rawPoint;

        // Print raw coordinates before calibration
        Serial.print("\n[RAW] X: ");
        Serial.print(rawPoint.x);
        Serial.print(", Y: ");
        Serial.println(rawPoint.y);

        // Calibrate to screen coordinates
        calibrateTouch(p);

        // Print calibrated coordinates
        Serial.print("[CAL] X: ");
        Serial.print(p.x);
        Serial.print(", Y: ");
        Serial.print(p.y);

        // Check if coordinates are within valid screen bounds
        if (p.x > SCREEN_WIDTH)
        {
            Serial.print(" [X OUT OF BOUNDS!]");
        }
        if (p.y > SCREEN_HEIGHT)
        {
            Serial.print(" [Y OUT OF BOUNDS!]");
        }
        Serial.println();

        // Check which button was pressed
        boolean inButton1 = isPointInButton(p, button1);
        boolean inButton2 = isPointInButton(p, button2);

        Serial.print("[CHECK] Button A (X ");
        Serial.print(button1.x);
        Serial.print("-");
        Serial.print(button1.x + button1.width);
        Serial.print(", Y ");
        Serial.print(button1.y);
        Serial.print("-");
        Serial.print(button1.y + button1.height);
        Serial.print(") -> ");
        Serial.println(inButton1 ? "HIT" : "MISS");

        Serial.print("[CHECK] Button2 (X ");
        Serial.print(button2.x);
        Serial.print("-");
        Serial.print(button2.x + button2.width);
        Serial.print(", Y ");
        Serial.print(button2.y);
        Serial.print("-");
        Serial.print(button2.y + button2.height);
        Serial.print(") -> ");
        Serial.println(inButton2 ? "HIT" : "MISS");

        if (inButton1)
        {
            displayMessage("Button 1!", TFT_GREEN);
            Serial.println(">>> BUTTON 1 ACTIVATED <<<");

            // Visual feedback
            tft.fillRect(button1.x, button1.y, button1.width, button1.height, TFT_CYAN);
            delay(200);
            drawButton(button1);
        }
        else if (inButton2)
        {
            displayMessage("Button 2!", TFT_GREEN);
            Serial.println(">>> BUTTON 2 ACTIVATED <<<");

            // Visual feedback
            tft.fillRect(button2.x, button2.y, button2.width, button2.height, TFT_CYAN);
            delay(200);
            drawButton(button2);
        }
        else
        {
            Serial.print("[NO BUTTON HIT] Touch was at screen coords: X=");
            Serial.print(p.x);
            Serial.print(", Y=");
            Serial.println(p.y);
        }

        // Wait for touch release
        delay(300);
    }
}
