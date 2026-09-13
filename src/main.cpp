#include <Arduino.h>
#include <lvgl.h>
#include <WiFi.h>
#include "Display.h"
#include "ClubScreen.h"
#include "SnapCallApi.h"
#include "config.h"

static const char *TAG = "[SNAPCALL]";
static SnapCallApi snapCallApi;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.printf("\n\n%s Starting SnapCall\n", TAG);
    
    Display::begin();
    
    // Connect to WiFi - try primary first, then secondary
    Serial.printf("%s Connecting to WiFi: %s\n", TAG, WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    // Try secondary WiFi if primary failed
    if (WiFi.status() != WL_CONNECTED && WIFI_SSID_2[0] != '\0') {
        Serial.printf("\n%s Primary WiFi failed. Trying secondary: %s\n", TAG, WIFI_SSID_2);
        WiFi.disconnect();
        delay(100);
        WiFi.begin(WIFI_SSID_2, WIFI_PASSWORD_2);
        
        attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.printf("\n%s WiFi connected! IP: %s\n", TAG, WiFi.localIP().toString().c_str());
    } else {
        Serial.printf("\n%s WARNING: WiFi connection failed - continuing offline\n", TAG);
    }
    
    // Show club selection screen
    ClubScreen::show(&snapCallApi, [](int clubId, const String &clubName) {
        Serial.printf("%s Club selected: %s (ID: %d)\n", TAG, clubName.c_str(), clubId);
        // TODO: Handle club selection - load tournaments, etc.
    });
    
    Serial.printf("%s Setup complete\n", TAG);
}

void loop()
{
    lv_tick_inc(10);  // Tick LVGL timer
    Display::update();
    delay(10);
}
