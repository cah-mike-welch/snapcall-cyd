#pragma once

#include <Arduino.h>

struct Blinds
{
    int smallBlind = 0;
    int bigBlind = 0;
    int bigBlindAnte = 0;
    String timeRemaining;
    bool isRunning = false;
    String levelType;
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

// Parses "MM:SS" or "HH:MM:SS" into total seconds. Returns 0 if unparsable.
inline int parseTimeRemainingToSeconds(const String &time)
{
    int colonCount = 0;
    for (unsigned int i = 0; i < time.length(); i++)
    {
        if (time[i] == ':')
        {
            colonCount++;
        }
    }

    int hours = 0, minutes = 0, seconds = 0;

    if (colonCount >= 2)
    {
        sscanf(time.c_str(), "%d:%d:%d", &hours, &minutes, &seconds);
    }
    else
    {
        sscanf(time.c_str(), "%d:%d", &minutes, &seconds);
    }

    return hours * 3600 + minutes * 60 + seconds;
}

// True if the time_remaining string uses "HH:MM:SS" (vs "MM:SS") so the countdown can preserve the format.
inline bool timeRemainingUsesHours(const String &time)
{
    int colonCount = 0;
    for (unsigned int i = 0; i < time.length(); i++)
    {
        if (time[i] == ':')
        {
            colonCount++;
        }
    }
    return colonCount >= 2;
}

// Formats total seconds back into "MM:SS" or "HH:MM:SS", matching the source format.
inline String formatSecondsToTimeRemaining(int totalSeconds, bool useHours)
{
    if (totalSeconds < 0)
    {
        totalSeconds = 0;
    }

    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    char buf[16];
    if (useHours)
    {
        snprintf(buf, sizeof(buf), "%d:%02d:%02d", hours, minutes, seconds);
    }
    else
    {
        snprintf(buf, sizeof(buf), "%d:%02d", minutes + hours * 60, seconds);
    }

    return String(buf);
}
