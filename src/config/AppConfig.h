#pragma once

#include <Arduino.h>

// Centralized hardware, network, and API configuration.
// Values are copied verbatim from the original main.cpp; do not change without updating hardware.

namespace AppConfig
{
    // TFT_BL and TFT_CS are already defined as macros via platformio.ini build_flags.

    // Touch controller (XPT2046) pins
    constexpr uint8_t kTouchIrqPin = 36;
    constexpr uint8_t kTouchMosiPin = 32;
    constexpr uint8_t kTouchMisoPin = 39;
    constexpr uint8_t kTouchClkPin = 25;
    constexpr uint8_t kTouchCsPin = 33;

    // Touch calibration
    constexpr uint16_t kTouchMinX = 300;
    constexpr uint16_t kTouchMaxX = 3800;
    constexpr uint16_t kTouchMinY = 400;
    constexpr uint16_t kTouchMaxY = 3800;

    // Screen dimensions
    constexpr uint16_t kScreenWidth = 240;
    constexpr uint16_t kScreenHeight = 320;

    // WiFi networks, tried in order via WiFiMulti
    struct WifiCredential
    {
        const char *ssid;
        const char *password;
    };

    constexpr WifiCredential kWifiNetworks[] = {
        {"370Katiebud", "mbebmike"},
        {"IHC of Cincinnati", "1r1shJig"},
    };

    // API
    constexpr const char *kApiBaseUrl = "https://snapcallapp.com/api/device/v1";
    constexpr const char *kBearerToken = "cRJyrW8Ep16fmBxIuQX7R0LUrMgIwQRdI87drWJEftzRajBrvkqujiPpjAQgSzpQ";
}
