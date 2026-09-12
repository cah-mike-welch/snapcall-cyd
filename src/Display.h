#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <lvgl.h>
#include <TFT_eSPI.h>

// ============================================================================
// DISPLAY MANAGEMENT
// ============================================================================

/**
 * Display class manages initialization and integration of:
 * - ILI9341 TFT display via TFT_eSPI
 * - LVGL UI framework
 * - XPT2046 touchscreen input
 */
class Display
{
public:
    /**
     * Initialize the display, LVGL, and touchscreen.
     */
    static void begin();

    /**
     * Call this regularly to process LVGL events and rendering.
     */
    static void update();

    /**
     * Get a reference to the TFT_eSPI display object.
     */
    static TFT_eSPI &getTFT();

    /**
     * Clear the screen and reset to default state.
     */
    static void clear();

private:
    static TFT_eSPI tft;
};

#endif // DISPLAY_H
