#pragma once

#include <functional>
#include <lvgl.h>
#include "models/Blinds.h"
#include "screens/NavBar.h"

// Builds the blinds display screen (table number, blinds, ante, time remaining, nav bar).
// Holds the time label so the countdown can update it once per second without rebuilding the screen.
class BlindsScreen
{
public:
    void show(const Blinds &blinds, int tableNumber, std::function<void()> onNavigateToPlayers);
    void updateTimeRemaining(const String &timeRemaining);

private:
    NavBar navBar_;
    lv_obj_t *timeLabel_ = nullptr;
};
