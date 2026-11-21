#pragma once

#include <functional>
#include <vector>
#include <lvgl.h>
#include "network/WifiManager.h"

// Touchscreen Wi-Fi setup: a "No WiFi" prompt, a list of scanned networks, and a password entry with an on-screen keyboard.
class WifiSetupScreen
{
public:
    void clear();
    void showNoWifi(std::function<void()> onSetup);
    void showMessage(const String &text);
    void showNetworkList(const std::vector<WifiScanResult> &networks,
                         std::function<void(const String &)> onSelect,
                         std::function<void()> onRescan,
                         std::function<void()> onBack);
    void showPasswordEntry(const String &ssid,
                           std::function<void(const String &)> onConnect,
                           std::function<void()> onCancel);
    void setStatus(const String &text);

private:
    static void networkRowEventCb(lv_event_t *e);
    static void actionEventCb(lv_event_t *e);
    static void connectEventCb(lv_event_t *e);
    static void runConnectAsync(void *userData);

    std::function<void()> onSetup_;
    std::function<void()> onRescan_;
    std::function<void()> onBack_;
    std::function<void()> onCancel_;
    std::function<void(const String &)> onSelect_;
    std::function<void(const String &)> onConnect_;

    std::vector<String> ssids_;
    lv_obj_t *statusLabel_ = nullptr;
    lv_obj_t *passwordField_ = nullptr;
};
