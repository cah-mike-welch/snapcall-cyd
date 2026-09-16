#pragma once

#include <Arduino.h>

struct Blinds
{
    int smallBlind = 0;
    int bigBlind = 0;
    int bigBlindAnte = 0;
    String timeRemaining;
};

// Matches the original format_blind_value(): e.g. 1000 -> "1K", 2500 -> "2.5K", 750 -> "750"
inline String formatBlindValue(int val)
{
    if (val < 1000)
    {
        return String(val);
    }
    if (val % 1000 == 0)
    {
        return String(val / 1000) + "K";
    }
    return String(val / 1000.0, 1) + "K";
}
