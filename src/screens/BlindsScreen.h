#pragma once

#include <functional>
#include <lvgl.h>
#include "models/Blinds.h"
#include "screens/NavBar.h"

// Builds the blinds display screen (table number, blinds, ante, time remaining, clock start/stop button, nav bar).
// Holds the time label so the countdown can update it once per second without rebuilding the screen.
class BlindsScreen
{
public:
    void show(const Blinds &blinds, int tableNumber, std::function<void()> onNavigateToPlayers, std::function<void()> onToggleClock);
    void updateTimeRemaining(const String &timeRemaining);

private:
    static void clockBtnEventCb(lv_event_t *e);

    NavBar navBar_;
    lv_obj_t *timeLabel_ = nullptr;
    std::function<void()> onToggleClock_;
};
