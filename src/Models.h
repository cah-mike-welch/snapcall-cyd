#ifndef MODELS_H
#define MODELS_H

#include <Arduino.h>

// ============================================================================
// DATA MODELS
// ============================================================================

/**
 * Club represents a poker club/venue.
 */
struct Club
{
    int id;
    String shortName;
    String name;

    Club() : id(0), shortName(""), name("") {}
    Club(int id, const String &shortName, const String &name)
        : id(id), shortName(shortName), name(name) {}
};

/**
 * BlindLevel represents the current blind structure.
 */
struct BlindLevel
{
    int smallBlind;
    int bigBlind;
    int bigBlindAnte;
    String timeRemaining; // Format: "MM:SS"

    BlindLevel() : smallBlind(0), bigBlind(0), bigBlindAnte(0), timeRemaining("00:00") {}
    BlindLevel(int sb, int bb, int ante, const String &time)
        : smallBlind(sb), bigBlind(bb), bigBlindAnte(ante), timeRemaining(time) {}
};

/**
 * TournamentState represents the current tournament and blind information.
 */
struct TournamentState
{
    int clubId;
    String clubName;
    BlindLevel blinds;
    unsigned long lastUpdatedMs;

    TournamentState() : clubId(0), clubName(""), lastUpdatedMs(0) {}
};

#endif // MODELS_H
