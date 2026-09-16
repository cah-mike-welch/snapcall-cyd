#include "WifiManager.h"
#include "config/AppConfig.h"

void WifiManager::begin()
{
    WiFi.mode(WIFI_STA);
    for (const auto &network : AppConfig::kWifiNetworks)
    {
        wifiMulti_.addAP(network.ssid, network.password);
    }

    Serial.print("Connecting to WiFi");
    while (wifiMulti_.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\n[WIFI] Connected!");
}

void WifiManager::maintainConnection()
{
    if (wifiMulti_.run() != WL_CONNECTED)
    {
        Serial.println("[WIFI] Connection lost! Attempting to reconnect...");
        delay(1000);
    }
}

bool WifiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WifiManager::localIP() const
{
    return WiFi.localIP();
}
