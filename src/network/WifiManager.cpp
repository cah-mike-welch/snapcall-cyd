#include "WifiManager.h"
#include <Preferences.h>
#include <algorithm>
#include "config/AppConfig.h"

namespace
{
    constexpr const char *kPrefsNamespace = "wifi";

    // Failures reported this soon after WiFi.begin() can be stale from the previous attempt.
    constexpr unsigned long kEarlyFailIgnoreMs = 1000;
    constexpr unsigned long kConnectPollMs = 100;

    struct Credential
    {
        String ssid;
        String password;
    };

    std::vector<Credential> readSavedCredentials()
    {
        std::vector<Credential> saved;
        Preferences prefs;
        prefs.begin(kPrefsNamespace, false);

        const uint8_t count = prefs.getUChar("count", 0);
        for (uint8_t i = 0; i < count && i < AppConfig::kWifiMaxSavedNetworks; i++)
        {
            String ssid = prefs.getString(("ssid" + String(i)).c_str());
            if (!ssid.isEmpty())
            {
                saved.push_back({ssid, prefs.getString(("pass" + String(i)).c_str())});
            }
        }

        prefs.end();
        return saved;
    }
}

bool WifiManager::begin()
{
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setSleep(false);
    lastRetryMs_ = millis();

    Serial.print("Connecting to WiFi");
    const bool connected = tryKnownNetworks();
    Serial.println(connected ? "\n[WIFI] Connected!" : "\n[WIFI] No known network available.");
    return connected;
}

void WifiManager::maintainConnection()
{
    if (isConnected() || millis() - lastRetryMs_ < AppConfig::kWifiRetryIntervalMs)
    {
        return;
    }

    lastRetryMs_ = millis();
    Serial.println("[WIFI] Not connected. Retrying known networks...");
    tryKnownNetworks();
}

std::vector<WifiScanResult> WifiManager::scan()
{
    std::vector<WifiScanResult> results;
    const int found = WiFi.scanNetworks();
    for (int i = 0; i < found; i++)
    {
        String ssid = WiFi.SSID(i);
        if (ssid.isEmpty())
        {
            continue;
        }

        const int32_t rssi = WiFi.RSSI(i);
        auto existing = std::find_if(results.begin(), results.end(),
                                     [&ssid](const WifiScanResult &r)
                                     { return r.ssid == ssid; });
        if (existing == results.end())
        {
            results.push_back({ssid, rssi});
        }
        else if (rssi > existing->rssi)
        {
            existing->rssi = rssi;
        }
    }
    WiFi.scanDelete();

    std::sort(results.begin(), results.end(),
              [](const WifiScanResult &a, const WifiScanResult &b)
              { return a.rssi > b.rssi; });
    return results;
}

bool WifiManager::connectTo(const String &ssid, const String &password)
{
    if (!attemptConnect(ssid, password, AppConfig::kWifiSetupConnectTimeoutMs))
    {
        return false;
    }

    saveNetwork(ssid, password);
    return true;
}

// Scans once, then tries only networks that are in range, strongest signal first.
// Saved networks come before the compiled-in list, so a saved password wins over a duplicate SSID.
bool WifiManager::tryKnownNetworks()
{
    std::vector<Credential> candidates = readSavedCredentials();
    for (const auto &network : AppConfig::kWifiNetworks)
    {
        candidates.push_back({network.ssid, network.password});
    }

    struct Visible
    {
        size_t index;
        int32_t rssi;
    };

    const int found = WiFi.scanNetworks();
    std::vector<Visible> visible;
    for (size_t c = 0; c < candidates.size(); c++)
    {
        bool duplicate = false;
        for (size_t j = 0; j < c && !duplicate; j++)
        {
            duplicate = candidates[j].ssid == candidates[c].ssid;
        }
        if (duplicate)
        {
            continue;
        }

        bool inRange = false;
        int32_t bestRssi = 0;
        for (int i = 0; i < found; i++)
        {
            if (WiFi.SSID(i) == candidates[c].ssid)
            {
                const int32_t rssi = WiFi.RSSI(i);
                bestRssi = inRange ? std::max(bestRssi, rssi) : rssi;
                inRange = true;
            }
        }
        if (inRange)
        {
            visible.push_back({c, bestRssi});
        }
    }
    WiFi.scanDelete();

    std::stable_sort(visible.begin(), visible.end(),
                     [](const Visible &a, const Visible &b)
                     { return a.rssi > b.rssi; });

    for (const auto &v : visible)
    {
        const Credential &cred = candidates[v.index];
        Serial.printf("[WIFI] Trying %s\n", cred.ssid.c_str());
        if (attemptConnect(cred.ssid, cred.password, AppConfig::kWifiConnectTimeoutMs))
        {
            return true;
        }
    }
    return false;
}

bool WifiManager::attemptConnect(const String &ssid, const String &password, unsigned long timeoutMs)
{
    WiFi.disconnect(false, false);
    delay(100);
    WiFi.begin(ssid.c_str(), password.c_str());

    const unsigned long start = millis();
    while (millis() - start < timeoutMs)
    {
        const wl_status_t status = WiFi.status();
        if (status == WL_CONNECTED)
        {
            return true;
        }

        // The AP is not visible or rejected us, so stop waiting for the full timeout.
        if (millis() - start > kEarlyFailIgnoreMs &&
            (status == WL_NO_SSID_AVAIL || status == WL_CONNECT_FAILED))
        {
            break;
        }
        delay(kConnectPollMs);
    }

    WiFi.disconnect(false, false);
    return false;
}

void WifiManager::saveNetwork(const String &ssid, const String &password)
{
    std::vector<Credential> saved = readSavedCredentials();
    saved.erase(std::remove_if(saved.begin(), saved.end(),
                               [&ssid](const Credential &c)
                               { return c.ssid == ssid; }),
                saved.end());
    saved.insert(saved.begin(), {ssid, password});
    if (saved.size() > AppConfig::kWifiMaxSavedNetworks)
    {
        saved.resize(AppConfig::kWifiMaxSavedNetworks);
    }

    Preferences prefs;
    prefs.begin(kPrefsNamespace, false);
    prefs.putUChar("count", static_cast<uint8_t>(saved.size()));
    for (size_t i = 0; i < saved.size(); i++)
    {
        prefs.putString(("ssid" + String(i)).c_str(), saved[i].ssid);
        prefs.putString(("pass" + String(i)).c_str(), saved[i].password);
    }
    prefs.end();
}

bool WifiManager::isConnected() const
{
    return WiFi.status() == WL_CONNECTED;
}

String WifiManager::ssid() const
{
    return WiFi.SSID();
}
