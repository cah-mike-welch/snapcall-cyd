// CYD LVGL + WiFi API + Dynamic UI (ESP32-2432S028R)
#include "Arduino.h"
#include "app/App.h"

App app;

void setup()
{
    app.begin();
}

void loop()
{
    app.loop();
}