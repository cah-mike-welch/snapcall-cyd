#pragma once

#include <lvgl.h>
#include "models/Blinds.h"

// Builds the blinds display screen (table number, blinds, ante, time remaining).
// Holds the time label so the countdown can update it once per second without rebuilding the screen.
class BlindsScreen
{
public:
    void show(const Blinds &blinds, int tableNumber);
    void updateTimeRemaining(const String &timeRemaining);

private:
    lv_obj_t *timeLabel_ = nullptr;
};
