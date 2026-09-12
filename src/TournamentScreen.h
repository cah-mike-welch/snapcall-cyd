#ifndef TOURNAMENT_SCREEN_H
#define TOURNAMENT_SCREEN_H

#include <Arduino.h>
#include <lvgl.h>
#include <functional>
#include "Models.h"

// Forward declaration
class SnapCallApi;

// ============================================================================
// TOURNAMENT STATUS SCREEN
// ============================================================================

/**
 * TournamentScreen displays the current blind level and time for a selected club.
 */
class TournamentScreen
{
public:
    /**
     * Create and display the tournament status screen.
     *
     * @param api Pointer to SnapCallApi for API calls.
     * @param clubId The ID of the club.
     * @param clubName The name of the club.
     * @param onBack Callback when user clicks Back.
     */
    static void show(SnapCallApi *api, int clubId, const String &clubName, std::function<void()> onBack);

    /**
     * Hide and destroy the tournament screen.
     */
    static void hide();

    /**
     * Update the tournament data (blinds, time remaining).
     */
    static void update();

private:
    static lv_obj_t *screen;
    static lv_obj_t *timeLabel;
    static lv_obj_t *smallBlindLabel;
    static lv_obj_t *bigBlindLabel;
    static lv_obj_t *anteLabel;
    static int currentClubId;
    static SnapCallApi *apiInstance;
    static unsigned long lastUpdateMs;
    static bool shouldRefresh;

    /**
     * Load and display tournament data from API.
     */
    static void loadTournamentData();

    /**
     * Handle Back button click.
     */
    static void onBackClicked(lv_event_t *e);
};

#endif // TOURNAMENT_SCREEN_H
