#ifndef CLUB_SCREEN_H
#define CLUB_SCREEN_H

#include <Arduino.h>
#include <lvgl.h>
#include <vector>
#include <functional>
#include "Models.h"

// Forward declaration
class SnapCallApi;

// ============================================================================
// CLUB SELECTION SCREEN
// ============================================================================

/**
 * ClubScreen displays the list of available clubs and handles club selection.
 */
class ClubScreen
{
public:
    /**
     * Create and display the club selection screen.
     * Callback is invoked when a club is selected.
     *
     * @param api Pointer to SnapCallApi for API calls.
     * @param onClubSelected Callback function with signature void(int clubId, const String& clubName).
     */
    static void show(SnapCallApi *api, std::function<void(int, const String &)> onClubSelected);

    /**
     * Hide and destroy the club selection screen.
     */
    static void hide();

private:
    static lv_obj_t *screen;
    static std::vector<Club> clubs;
    static std::function<void(int, const String &)> selectedCallback;
    static SnapCallApi *apiInstance;
    static bool isLoading;

    /**
     * Load clubs from API and display them.
     */
    static void loadClubs();

    /**
     * Handle club button click.
     */
    static void onClubClicked(lv_event_t *e);

    /**
     * Display error state.
     */
    static void showError(const String &message);
};

#endif // CLUB_SCREEN_H
