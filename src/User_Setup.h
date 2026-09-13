/*
 * User_Setup.h for TFT_eSPI - esp32-2432s028 (Cheap Yellow Display)
 * This file configures TFT_eSPI for ILI9341 display with XPT2046 touchscreen
 */

#ifndef _USER_SETUP_H_
#define _USER_SETUP_H_

// Driver selection
#define ILI9341_DRIVER

// Display pins (SPI)
#define TFT_MOSI 13
#define TFT_MISO 12
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC   2
#define TFT_RST  -1

// Backlight
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// Display resolution
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// Invert display
#define TFT_INVERSION_ON

// Touch screen XPT2046
#define TOUCH_CS 33
#define TOUCH_CLK   14
#define TOUCH_MOSI  13
#define TOUCH_MISO  12
#define TOUCH_IRQ   36  // GPIO 36 (VP pin) - interrupt input from XPT2046

// Touch is on same SPI bus as display
#define TOUCH_SPI_PORT VSPI_HOST

// SPI frequencies
#define SPI_FREQUENCY       55000000  // Display SPI speed
#define SPI_READ_FREQUENCY  20000000  // Read SPI speed  
#define SPI_TOUCH_FREQUENCY 2500000   // Touch SPI speed (lower for stability)

// Touch calibration ranges (raw sensor values)
#define TOUCH_CALIBRATE_X 320
#define TOUCH_CALIBRATE_Y 240

// Font loading
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

// Smooth fonts
#define SMOOTH_FONT

// Specify ESP32 build
#define TFT_SPI_PORT VSPI_HOST

#endif // _USER_SETUP_H_
