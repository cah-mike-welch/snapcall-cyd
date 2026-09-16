#pragma once

#include <WiFi.h>
#include <WiFiMulti.h>

// Wraps WiFiMulti setup/connect/reconnect so the rest of the app never touches WiFiMulti directly.
class WifiManager
{
public:
    // Configures WIFI_STA, adds the configured APs, and blocks until connected.
    void begin();

    // Call from loop(); reconnects if the connection was lost.
    void maintainConnection();

    bool isConnected() const;
    IPAddress localIP() const;

private:
    WiFiMulti wifiMulti_;
};
