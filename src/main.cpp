#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "Display.h"
#include "SnapCallApi.h"
#include "ClubScreen.h"
#include "TournamentScreen.h"

static const char *TAG = "[BOOT]";

// Global instances
SnapCallApi api;
bool wifiConnected = false;

// Application state
enum AppState
{
    STATE_INIT,
    STATE_WIFI_CONNECTING,
    STATE_CLUB_SELECTION,
    STATE_LOADING_TOURNAMENT,
    STATE_TOURNAMENT_VIEW,
    STATE_ERROR
};

static AppState currentState = STATE_INIT;
static int selectedClubId = 0;
static String selectedClubName = "";
static bool clubScreenShown = false;
static bool tournamentScreenShown = false;

// Forward declarations
void stateInit();
void stateWifiConnecting();
void stateClubSelection();
void stateLoadingTournament();
void stateTournamentView();
void stateError(const String &message);

void showLoadingScreen(const String &message)
{
    Display::clear();
    lv_obj_t *screen = lv_scr_act();

    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, message.c_str());
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void onClubSelected(int clubId, const String &clubName)
{
    Serial.printf("%s Club selected: %s (ID: %d)\n", TAG, clubName.c_str(), clubId);
    selectedClubId = clubId;
    selectedClubName = clubName;
    currentState = STATE_LOADING_TOURNAMENT;
}

void onTournamentBack()
{
    Serial.printf("%s Returning to club selection\n", TAG);
    clubScreenShown = false;
    currentState = STATE_CLUB_SELECTION;
}

void onRetryError(lv_event_t *e)
{
    currentState = STATE_INIT;
    clubScreenShown = false;
    tournamentScreenShown = false;
}

void setup()
{
    Serial.begin(SERIAL_BAUD_RATE);
    delay(500);

    Serial.printf("\n%s SnapCall Dealer Button - Starting\n", TAG);

    // Initialize display
    Display::begin();
    Serial.printf("%s Display initialized\n", TAG);

    // Initialize API
    api.begin();

    // Initialize state machine
    currentState = STATE_INIT;
}

void loop()
{
    // Update display
    Display::update();

    // Update tournament screen if active
    if (currentState == STATE_TOURNAMENT_VIEW)
    {
        TournamentScreen::update();
    }

    // State machine
    switch (currentState)
    {
    case STATE_INIT:
        stateInit();
        break;
    case STATE_WIFI_CONNECTING:
        stateWifiConnecting();
        break;
    case STATE_CLUB_SELECTION:
        stateClubSelection();
        break;
    case STATE_LOADING_TOURNAMENT:
        stateLoadingTournament();
        break;
    case STATE_TOURNAMENT_VIEW:
        stateTournamentView();
        break;
    case STATE_ERROR:
        // Error state - wait for button press to retry
        break;
    default:
        break;
    }

    delay(50); // Yield to FreeRTOS
}

void stateInit()
{
    Serial.printf("%s Initializing application\n", TAG);
    showLoadingScreen("Connecting to WiFi...");
    currentState = STATE_WIFI_CONNECTING;
}

void stateWifiConnecting()
{
    static unsigned long startTime = 0;
    static bool connecting = false;
    static const unsigned long WIFI_TIMEOUT = 20000; // 20 seconds

    if (!connecting)
    {
        Serial.printf("%s Connecting to WiFi: %s\n", TAG, WIFI_SSID);
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        connecting = true;
        startTime = millis();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.printf("%s WiFi connected!\n", TAG);
        Serial.printf("%s IP: %s\n", TAG, WiFi.localIP().toString().c_str());
        wifiConnected = true;
        connecting = false;
        currentState = STATE_CLUB_SELECTION;
    }
    else if (millis() - startTime > WIFI_TIMEOUT)
    {
        Serial.printf("%s WiFi connection timeout\n", TAG);
        stateError("WiFi connection failed");
        connecting = false;
    }
}

void stateClubSelection()
{
    if (!clubScreenShown)
    {
        Serial.printf("%s Showing club selection screen\n", TAG);
        ClubScreen::show(&api, onClubSelected);
        clubScreenShown = true;
    }

    // Screen is now showing and waiting for user input
    // When user taps, onClubSelected callback will transition state
}

void stateLoadingTournament()
{
    Serial.printf("%s Loading tournament data for club: %s\n", TAG, selectedClubName.c_str());
    showLoadingScreen("Loading tournament...");

    // Simulate async loading
    delay(500);

    tournamentScreenShown = false;
    currentState = STATE_TOURNAMENT_VIEW;
}

void stateTournamentView()
{
    if (!tournamentScreenShown)
    {
        Serial.printf("%s Showing tournament screen\n", TAG);
        ClubScreen::hide();
        TournamentScreen::show(&api, selectedClubId, selectedClubName, onTournamentBack);
        tournamentScreenShown = true;
    }

    // Screen is now showing, TournamentScreen::update() handles periodic refresh
}

void stateError(const String &message)
{
    currentState = STATE_ERROR;

    Serial.printf("%s ERROR: %s\n", TAG, message.c_str());

    Display::clear();
    lv_obj_t *screen = lv_scr_act();

    lv_obj_t *errorLabel = lv_label_create(screen);
    lv_label_set_text(errorLabel, message.c_str());
    lv_obj_set_style_text_font(errorLabel, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(errorLabel, lv_color_hex(0xFF6666), LV_PART_MAIN);
    lv_obj_align(errorLabel, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *retryBtn = lv_btn_create(screen);
    lv_obj_set_size(retryBtn, 100, 40);
    lv_obj_align(retryBtn, LV_ALIGN_CENTER, 0, 30);
    lv_obj_set_style_bg_color(retryBtn, lv_color_hex(0x3a3a3a), LV_PART_MAIN);

    lv_obj_t *retryLabel = lv_label_create(retryBtn);
    lv_label_set_text(retryLabel, "Retry");
    lv_obj_set_style_text_color(retryLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_center(retryLabel);

    lv_obj_add_event_cb(retryBtn, onRetryError, LV_EVENT_CLICKED, NULL);
}
