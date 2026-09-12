#ifndef CONFIG_EXAMPLE_H
#define CONFIG_EXAMPLE_H

// ============================================================================
// WIFI CONFIGURATION
// ============================================================================
// Replace with your actual Wi-Fi credentials.
// Do NOT commit the real config.h to Git.

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// ============================================================================
// SNAPCALL API CONFIGURATION
// ============================================================================
// Replace with your SnapCall API endpoint and API key.

#define SNAPCALL_API_BASE_URL "http://snapcallapp.com/api/device/v1"
#define SNAPCALL_API_KEY "YOUR_API_KEY"

// ============================================================================
// SERIAL LOGGING
// ============================================================================
// Serial baud rate for debug output.

#define SERIAL_BAUD_RATE 115200

// ============================================================================
// DISPLAY CONFIGURATION
// ============================================================================
// Landscape orientation: 320x240

#define DISPLAY_WIDTH 320
#define DISPLAY_HEIGHT 240

// ============================================================================
// API POLLING CONFIGURATION
// ============================================================================
// How often to refresh tournament/blind data (milliseconds)

#define REFRESH_INTERVAL_MS 12000 // 12 seconds

#endif // CONFIG_EXAMPLE_H
