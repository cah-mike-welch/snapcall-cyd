#pragma once

#include <functional>
#include <lvgl.h>
#include "models/Blinds.h"
#include "screens/NavBar.h"

// Builds the blinds display screen (table number, blinds, ante, time remaining, battery voltage, clock start/stop button, nav bar).
// Holds the time and battery labels so the countdown and battery reading can update them without rebuilding the screen.
class BlindsScreen
{
public:
    void show(const Blinds &blinds, int tableNumber, int batteryPercent, std::function<void()> onNavigateToPlayers, std::function<void()> onToggleClock);
    void updateTimeRemaining(const String &timeRemaining);
    void updateBatteryLevel(int batteryPercent);

private:
    static void clockBtnEventCb(lv_event_t *e);

    NavBar navBar_;
    lv_obj_t *timeLabel_ = nullptr;
    lv_obj_t *batteryLabel_ = nullptr;
    std::function<void()> onToggleClock_;
};
