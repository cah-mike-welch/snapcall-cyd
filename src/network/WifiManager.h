#pragma once

#include <WiFi.h>
#include <vector>

struct WifiScanResult
{
    String ssid;
    int32_t rssi;
};

// Owns Wi-Fi connection logic: compiled-in and user-saved networks, scanning, timed connects, and retries.
class WifiManager
{
public:
    // Starts STA mode and makes one pass over the known networks. Returns true if connected.
    bool begin();

    // Call from loop(); retries known networks on an interval while disconnected.
    void maintainConnection();

    // Blocking scan. Returns unique SSIDs, strongest first.
    std::vector<WifiScanResult> scan();

    // Blocking connect with a longer timeout. Saves the network to flash on success.
    bool connectTo(const String &ssid, const String &password);

    bool isConnected() const;
    String ssid() const;

private:
    bool tryKnownNetworks();
    bool attemptConnect(const String &ssid, const String &password, unsigned long timeoutMs);
    void saveNetwork(const String &ssid, const String &password);

    unsigned long lastRetryMs_ = 0;
};
