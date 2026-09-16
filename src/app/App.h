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

enum class AppState
{
    ClubSelection,
    TableSelection,
    Blinds
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

    void onClubSelected(const Club &club);
    void onTableSelected(const Table &table);

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

    lv_obj_t *labelStatus_ = nullptr;
    lv_obj_t *labelWifi_ = nullptr;
};
