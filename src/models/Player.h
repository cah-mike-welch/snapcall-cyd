#pragma once

#include <Arduino.h>

struct Player
{
    int id = 0;
    String firstName;
    String lastName;
    int seatNumber = 0;
};

// "S1\nR Bussell" for an occupied seat (seat number, first initial + last name truncated to 7 chars),
// or "S5\nOpen" for an empty seat.
inline String formatPlayerLabel(const Player &player)
{
    if (player.firstName.length() == 0)
    {
        char buf[16];
        snprintf(buf, sizeof(buf), "S%d\nOpen", player.seatNumber);
        return String(buf);
    }

    String lastName = player.lastName;
    if (lastName.length() > 7)
    {
        lastName = lastName.substring(0, 7);
    }

    char seatBuf[8];
    snprintf(seatBuf, sizeof(seatBuf), "S%d\n", player.seatNumber);
    return String(seatBuf) + player.firstName[0] + " " + lastName;
}
