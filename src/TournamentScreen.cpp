#include "TournamentScreen.h"
#include "SnapCallApi.h"
#include "config.h"

static const char *TAG = "[UI-TOURNAMENT]";

// Static member initialization
lv_obj_t *TournamentScreen::screen = nullptr;
lv_obj_t *TournamentScreen::timeLabel = nullptr;
lv_obj_t *TournamentScreen::smallBlindLabel = nullptr;
lv_obj_t *TournamentScreen::bigBlindLabel = nullptr;
lv_obj_t *TournamentScreen::anteLabel = nullptr;
int TournamentScreen::currentClubId = 0;
SnapCallApi *TournamentScreen::apiInstance = nullptr;
unsigned long TournamentScreen::lastUpdateMs = 0;
bool TournamentScreen::shouldRefresh = false;
std::function<void()> backCallback;

void TournamentScreen::show(SnapCallApi *api, int clubId, const String &clubName, std::function<void()> onBack)
{
    Serial.printf("%s Showing tournament screen for club: %s\n", TAG, clubName.c_str());

    apiInstance = api;
    currentClubId = clubId;
    backCallback = onBack;
    lastUpdateMs = 0;
    shouldRefresh = true;

    // Create screen
    screen = lv_obj_create(NULL);
    lv_scr_load(screen);
    lv_obj_set_size(screen, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

    // Header with club name
    lv_obj_t *headerLabel = lv_label_create(screen);
    lv_label_set_text(headerLabel, clubName.c_str());
    lv_obj_set_style_text_font(headerLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(headerLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(headerLabel, LV_ALIGN_TOP_MID, 0, 5);

    // BLINDS label
    lv_obj_t *blindsTitle = lv_label_create(screen);
    lv_label_set_text(blindsTitle, "BLINDS");
    lv_obj_set_style_text_font(blindsTitle, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(blindsTitle, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_align(blindsTitle, LV_ALIGN_TOP_MID, 0, 35);

    // Blind levels (SB / BB)
    lv_obj_t *blindsContainer = lv_obj_create(screen);
    lv_obj_set_size(blindsContainer, DISPLAY_WIDTH, 50);
    lv_obj_align(blindsContainer, LV_ALIGN_TOP_MID, 0, 55);
    lv_obj_set_style_border_width(blindsContainer, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(blindsContainer, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_flex_flow(blindsContainer, LV_FLEX_FLOW_COLUMN);

    smallBlindLabel = lv_label_create(blindsContainer);
    lv_label_set_text(smallBlindLabel, "Loading...");
    lv_obj_set_style_text_font(smallBlindLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(smallBlindLabel, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(smallBlindLabel, LV_ALIGN_CENTER, 0, -10);

    bigBlindLabel = lv_label_create(blindsContainer);
    lv_label_set_text(bigBlindLabel, "");
    lv_obj_set_style_text_font(bigBlindLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(bigBlindLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(bigBlindLabel, LV_ALIGN_CENTER, 0, 15);

    // BB Ante label
    anteLabel = lv_label_create(screen);
    lv_label_set_text(anteLabel, "");
    lv_obj_set_style_text_font(anteLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(anteLabel, lv_color_hex(0xFFDD00), LV_PART_MAIN);
    lv_obj_align(anteLabel, LV_ALIGN_TOP_MID, 0, 115);

    // TIME label
    lv_obj_t *timeTitle = lv_label_create(screen);
    lv_label_set_text(timeTitle, "TIME REMAINING");
    lv_obj_set_style_text_font(timeTitle, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(timeTitle, lv_color_hex(0xAAAAAA), LV_PART_MAIN);
    lv_obj_align(timeTitle, LV_ALIGN_CENTER, 0, 140);

    // Time display
    timeLabel = lv_label_create(screen);
    lv_label_set_text(timeLabel, "--:--");
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0xFF4444), LV_PART_MAIN);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, 170);

    // Back button
    lv_obj_t *backBtn = lv_btn_create(screen);
    lv_obj_set_size(backBtn, 80, 35);
    lv_obj_align(backBtn, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_color(backBtn, lv_color_hex(0x3a3a3a), LV_PART_MAIN);
    lv_obj_set_style_bg_color(backBtn, lv_color_hex(0x5a5a5a), LV_STATE_PRESSED);

    lv_obj_t *backLabel = lv_label_create(backBtn);
    lv_label_set_text(backLabel, "BACK");
    lv_obj_set_style_text_color(backLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(backLabel);

    lv_obj_add_event_cb(backBtn, onBackClicked, LV_EVENT_CLICKED, NULL);

    // Load initial tournament data
    loadTournamentData();
}

void TournamentScreen::hide()
{
    shouldRefresh = false;
    if (screen != nullptr)
    {
        lv_obj_del(screen);
        screen = nullptr;
        timeLabel = nullptr;
        smallBlindLabel = nullptr;
        bigBlindLabel = nullptr;
        anteLabel = nullptr;
    }
}

void TournamentScreen::update()
{
    if (!shouldRefresh || screen == nullptr || apiInstance == nullptr)
    {
        return;
    }

    unsigned long now = millis();
    if (now - lastUpdateMs >= REFRESH_INTERVAL_MS)
    {
        loadTournamentData();
        lastUpdateMs = now;
    }
}

void TournamentScreen::loadTournamentData()
{
    Serial.printf("%s Loading tournament data for club %d\n", TAG, currentClubId);

    if (apiInstance == nullptr)
    {
        return;
    }

    BlindLevel blinds = apiInstance->getCurrentBlinds(currentClubId);

    if (blinds.timeRemaining.isEmpty() || blinds.timeRemaining == "00:00")
    {
        Serial.printf("%s No active tournament\n", TAG);
        if (timeLabel)
        {
            lv_label_set_text(timeLabel, "NO TOURNAMENT");
        }
        return;
    }

    // Update blind labels
    String blindsDisplay = String(blinds.smallBlind) + " / " + String(blinds.bigBlind);
    if (smallBlindLabel)
    {
        lv_label_set_text(smallBlindLabel, blindsDisplay.c_str());
    }

    if (bigBlindLabel)
    {
        lv_label_set_text(bigBlindLabel, "");
    }

    // Update ante label
    String anteDisplay = "BB Ante: " + String(blinds.bigBlindAnte);
    if (anteLabel)
    {
        lv_label_set_text(anteLabel, anteDisplay.c_str());
    }

    // Update time label
    if (timeLabel)
    {
        lv_label_set_text(timeLabel, blinds.timeRemaining.c_str());
    }

    Serial.printf("%s Tournament updated: %s/%s ante %s time %s\n", TAG,
                  String(blinds.smallBlind).c_str(),
                  String(blinds.bigBlind).c_str(),
                  String(blinds.bigBlindAnte).c_str(),
                  blinds.timeRemaining.c_str());
}

void TournamentScreen::onBackClicked(lv_event_t *e)
{
    Serial.printf("%s Back button clicked\n", TAG);

    if (backCallback)
    {
        hide();
        backCallback();
    }
}
