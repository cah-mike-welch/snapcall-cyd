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

    // Battery sense on GPIO35 through a 1:1 100K/100K divider, so battery volts = pin volts x 2.
    constexpr uint8_t kBatteryAdcPin = 35;
    constexpr float kBatteryVoltageScale = 2.0f;
    constexpr uint8_t kBatteryAdcSamples = 8;

    // Screen dimensions
    constexpr uint16_t kScreenWidth = 240;
    constexpr uint16_t kScreenHeight = 320;

    // WiFi networks compiled into the firmware. Passwords saved from the setup screen are kept in flash and tried first.
    struct WifiCredential
    {
        const char *ssid;
        const char *password;
    };

    constexpr unsigned long kWifiConnectTimeoutMs = 10000;       // per known network at boot and on retry
    constexpr unsigned long kWifiSetupConnectTimeoutMs = 20000;  // network entered on the setup screen
    constexpr unsigned long kWifiRetryIntervalMs = 30000;        // how often to retry while disconnected
    constexpr uint8_t kWifiMaxSavedNetworks = 8;

    constexpr WifiCredential kWifiNetworks[] = {
        // {"370Katiebud", "mbebmike"},  // temporarily disabled to test the WiFi setup screen
        {"IHC of Cincinnati", "1r1shJig"},
        {"milford", "5133255070"},
        {"Gibby House 2", "Maisy2019"},
        {"Mike's iPhone 12", "mikemike"},
    };

    // API
    constexpr const char *kApiBaseUrl = "https://snapcallapp.com/api/device/v1";
    constexpr const char *kBearerToken = "cRJyrW8Ep16fmBxIuQX7R0LUrMgIwQRdI87drWJEftzRajBrvkqujiPpjAQgSzpQ";
}
