#include "App.h"

#include <Arduino.h>

namespace
{
    constexpr unsigned long kBlindsTickIntervalMs = 1000;
    constexpr unsigned long kBlindsResyncIntervalMs = 10000;
}

// ==========================================
// BASE UI
// ==========================================
// Builds the static text elements (WiFi IP and Status bar)
void App::createBaseUi()
{
    displayManager_.setDarkBackground();

    labelWifi_ = lv_label_create(lv_scr_act());

    // Set font size 14 explicitly for Wi-Fi info
    lv_obj_set_style_text_font(labelWifi_, &lv_font_montserrat_14, 0);

    if (wifiManager_.isConnected())
    {
        String wifiInfo = "WiFi: Connected\nIP: " + wifiManager_.localIP().toString();
        lv_label_set_text(labelWifi_, wifiInfo.c_str());
    }
    else
    {
        lv_label_set_text(labelWifi_, "WiFi Disconnected");
    }

    // White text color for Wi-Fi details
    lv_obj_set_style_text_color(labelWifi_, lv_color_white(), 0);

    lv_obj_set_style_text_align(labelWifi_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(labelWifi_, LV_ALIGN_TOP_MID, 0, 5);

    labelStatus_ = lv_label_create(lv_scr_act());
    lv_label_set_text(labelStatus_, "Fetching data...");
    lv_obj_set_style_text_color(labelStatus_, lv_color_white(), 0);
    lv_obj_align(labelStatus_, LV_ALIGN_BOTTOM_LEFT, 10, -10);
}

// ==========================================
// BLINDS
// ==========================================
void App::fetchBlindsAndShowScreen()
{
    if (!wifiManager_.isConnected())
    {
        lv_label_set_text(labelStatus_, "Error: No WiFi");
        return;
    }

    tableScreen_.reset();
    lv_obj_clean(lv_scr_act());
    displayManager_.setDarkBackground();

    // Note: "Blinds Live." status label has been removed from this screen entirely.

    Blinds blinds;
    ApiResult result = apiClient_.getCurrentBlinds(selectedClubId_, blinds);

    if (result == ApiResult::Success)
    {
        currentBlinds_ = blinds;
        blindsRemainingSeconds_ = parseTimeRemainingToSeconds(blinds.timeRemaining);
        blindsTimeUsesHours_ = timeRemainingUsesHours(blinds.timeRemaining);
        blindsLastTickMs_ = millis();
        blindsLastResyncMs_ = millis();

        blindsScreen_.show(blinds, selectedTableNumber_, [this]
                           { fetchPlayersAndShowScreen(); });
    }
    else
    {
        Serial.println("[API] Failed to fetch blinds");
    }

    state_ = AppState::Blinds;
}

// Re-fetches blinds from the API (source of truth) and refreshes the screen.
void App::resyncBlinds()
{
    if (!wifiManager_.isConnected())
    {
        return;
    }

    Blinds blinds;
    ApiResult result = apiClient_.getCurrentBlinds(selectedClubId_, blinds);

    if (result != ApiResult::Success)
    {
        Serial.println("[API] Failed to resync blinds");
        return;
    }

    currentBlinds_ = blinds;
    blindsRemainingSeconds_ = parseTimeRemainingToSeconds(blinds.timeRemaining);
    blindsTimeUsesHours_ = timeRemainingUsesHours(blinds.timeRemaining);
    blindsLastTickMs_ = millis();

    lv_obj_clean(lv_scr_act());
    displayManager_.setDarkBackground();
    blindsScreen_.show(blinds, selectedTableNumber_, [this]
                       { fetchPlayersAndShowScreen(); });
}

// Ticks the countdown once per second (only while the clock is running) and resyncs from the API periodically.
void App::updateBlindsCountdown()
{
    unsigned long now = millis();

    if (now - blindsLastResyncMs_ >= kBlindsResyncIntervalMs)
    {
        blindsLastResyncMs_ = now;
        resyncBlinds();
        return;
    }

    if (!currentBlinds_.isRunning)
    {
        blindsLastTickMs_ = now;
        return;
    }

    if (now - blindsLastTickMs_ >= kBlindsTickIntervalMs)
    {
        blindsLastTickMs_ += kBlindsTickIntervalMs;

        if (blindsRemainingSeconds_ > 0)
        {
            blindsRemainingSeconds_--;
        }

        blindsScreen_.updateTimeRemaining(formatSecondsToTimeRemaining(blindsRemainingSeconds_, blindsTimeUsesHours_));
    }
}

// ==========================================
// PLAYERS
// ==========================================
void App::fetchPlayersAndShowScreen()
{
    if (!wifiManager_.isConnected())
    {
        return;
    }

    lv_obj_clean(lv_scr_act());
    displayManager_.setDarkBackground();

    std::vector<Player> players;
    ApiResult result = apiClient_.getPlayers(selectedClubId_, selectedTableId_, players);

    if (result == ApiResult::Success)
    {
        playersScreen_.show(players, [this]
                            { fetchBlindsAndShowScreen(); }, [this](const Player &player)
                            { onPlayerSelected(player); });
    }
    else
    {
        Serial.println("[API] Failed to fetch players");
    }

    state_ = AppState::Players;
}

// ==========================================
// CONFIRM ELIMINATE
// ==========================================
void App::onPlayerSelected(const Player &player)
{
    selectedPlayerForElimination_ = player;

    lv_obj_clean(lv_scr_act());
    displayManager_.setDarkBackground();

    confirmEliminateScreen_.show(
        player,
        [this]
        { onEliminateConfirmed(); },
        [this]
        { onEliminateCancelled(); });

    state_ = AppState::ConfirmEliminate;
}

void App::onEliminateConfirmed()
{
    ApiResult result = apiClient_.eliminatePlayer(selectedClubId_, selectedPlayerForElimination_.id);

    if (result != ApiResult::Success)
    {
        Serial.println("[API] Failed to eliminate player");
    }

    fetchPlayersAndShowScreen();
}

void App::onEliminateCancelled()
{
    fetchPlayersAndShowScreen();
}

// ==========================================
// TABLE SELECTION
// ==========================================
void App::onTableSelected(const Table &table)
{
    selectedTournamentId_ = table.tournamentId;
    selectedTableId_ = table.id;
    selectedTableNumber_ = table.tableNumber;

    Serial.println("\n--- Table Selected ---");
    Serial.printf("Tournament ID: %d\n", selectedTournamentId_);
    Serial.printf("Table ID: %d\n", selectedTableId_);
    Serial.printf("Table Number: %d\n", selectedTableNumber_);

    fetchBlindsAndShowScreen();
}

void App::fetchTablesAndShowScreen()
{
    if (!wifiManager_.isConnected())
    {
        lv_label_set_text(labelStatus_, "Error: No WiFi");
        return;
    }

    clubScreen_.reset();
    lv_obj_clean(lv_scr_act());
    createBaseUi();

    lv_label_set_text(labelStatus_, "Fetching tables...");

    Tournament tournament;
    ApiResult result = apiClient_.getCurrentTournament(selectedClubId_, tournament);

    if (result == ApiResult::Success)
    {
        tableScreen_.reset(new TableSelectionScreen());
        tableScreen_->show(tournament, [this](const Table &table)
                           { onTableSelected(table); });

        lv_label_set_text(labelStatus_, "Select a table.");
        lv_obj_set_style_text_color(labelStatus_, lv_color_white(), 0);
    }
    else if (result == ApiResult::ParseError)
    {
        Serial.println("[API] JSON Parse failed for tables");
        lv_label_set_text(labelStatus_, "Parse Error");
    }
    else
    {
        Serial.println("[API] GET tables failed");
        lv_label_set_text(labelStatus_, "API Error");
    }

    state_ = AppState::TableSelection;
}

// ==========================================
// CLUB SELECTION
// ==========================================
void App::onClubSelected(const Club &club)
{
    selectedClubId_ = club.id;
    selectedClubShortName_ = club.shortName;
    selectedClubFullName_ = club.name;

    Serial.println("\n--- Club Selected ---");
    Serial.printf("ID: %d\n", selectedClubId_);
    Serial.printf("Short Name: %s\n", selectedClubShortName_.c_str());
    Serial.printf("Full Name: %s\n", selectedClubFullName_.c_str());

    char status_buf[64];
    snprintf(status_buf, sizeof(status_buf), "Active: %s", selectedClubShortName_.c_str());
    lv_label_set_text(labelStatus_, status_buf);
    lv_obj_set_style_text_color(labelStatus_, lv_color_white(), 0);

    fetchTablesAndShowScreen();
}

void App::fetchClubsAndShowScreen()
{
    if (!wifiManager_.isConnected())
    {
        lv_label_set_text(labelStatus_, "Error: No WiFi");
        return;
    }

    std::vector<Club> clubs;
    ApiResult result = apiClient_.getClubs(clubs);

    if (result != ApiResult::Success)
    {
        Serial.println("[API] Failed to fetch clubs");
        return;
    }

    clubScreen_.reset(new ClubSelectionScreen());
    clubScreen_->show(clubs, [this](const Club &club)
                      { onClubSelected(club); });

    lv_label_set_text(labelStatus_, "Ready.");
    lv_obj_set_style_text_color(labelStatus_, lv_color_white(), 0);

    state_ = AppState::ClubSelection;
}

// ==========================================
// LIFECYCLE
// ==========================================
void App::begin()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("\n--- Initializing ESP32 CYD ---");

    wifiManager_.begin();
    displayManager_.begin();
    touchManager_.begin();

    createBaseUi();
    fetchClubsAndShowScreen();

    Serial.println("[SYSTEM] Setup complete.");
}

void App::loop()
{
    wifiManager_.maintainConnection();

    if (state_ == AppState::Blinds)
    {
        updateBlindsCountdown();
    }

    lv_timer_handler();
    delay(5);
}
