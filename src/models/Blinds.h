#pragma once

#include <Arduino.h>

struct Blinds
{
    // Already formatted for display by the API (e.g. "800" or "1.6K"); shown as-is, not reformatted.
    String smallBlind;
    String bigBlind;
    String bigBlindAnte;
    String timeRemaining;
    bool isRunning = false;
    String levelType;
};

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
