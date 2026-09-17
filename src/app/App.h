#pragma once

#include <lvgl.h>
#include <memory>
#include "network/WifiManager.h"
#include "api/ApiClient.h"
#include "display/DisplayManager.h"
#include "display/TouchManager.h"
#include "screens/ClubSelectionScreen.h"
#include "screens/TableSelectionScreen.h"
#include "screens/BlindsScreen.h"
#include "screens/PlayersScreen.h"
#include "screens/ConfirmEliminateScreen.h"

enum class AppState
{
    ClubSelection,
    TableSelection,
    Blinds,
    Players,
    ConfirmEliminate
};

// Coordinates WiFi, the API client, the display/touch hardware, and screen navigation.
class App
{
public:
    void begin();
    void loop();

private:
    void createBaseUi();
    void fetchClubsAndShowScreen();
    void fetchTablesAndShowScreen();
    void fetchBlindsAndShowScreen();
    void fetchPlayersAndShowScreen();
    void resyncBlinds();
    void updateBlindsCountdown();

    void onClubSelected(const Club &club);
    void onTableSelected(const Table &table);
    void onPlayerSelected(const Player &player);
    void onEliminateConfirmed();
    void onEliminateCancelled();

    WifiManager wifiManager_;
    ApiClient apiClient_;
    DisplayManager displayManager_;
    TouchManager touchManager_;

    AppState state_ = AppState::ClubSelection;

    int selectedClubId_ = 0;
    String selectedClubShortName_;
    String selectedClubFullName_;

    int selectedTournamentId_ = 0;
    int selectedTableId_ = 0;
    int selectedTableNumber_ = 0;

    std::unique_ptr<ClubSelectionScreen> clubScreen_;
    std::unique_ptr<TableSelectionScreen> tableScreen_;
    BlindsScreen blindsScreen_;
    PlayersScreen playersScreen_;
    ConfirmEliminateScreen confirmEliminateScreen_;
    Player selectedPlayerForElimination_;

    // Blinds countdown: server is the source of truth, resynced every kBlindsResyncIntervalMs.
    Blinds currentBlinds_;
    int blindsRemainingSeconds_ = 0;
    bool blindsTimeUsesHours_ = false;
    unsigned long blindsLastTickMs_ = 0;
    unsigned long blindsLastResyncMs_ = 0;

    lv_obj_t *labelStatus_ = nullptr;
    lv_obj_t *labelWifi_ = nullptr;
};
