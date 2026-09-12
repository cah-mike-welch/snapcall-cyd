#include "ClubScreen.h"
#include "SnapCallApi.h"
#include "config.h"
#include <cstdint>

static const char *TAG = "[UI-CLUBS]";

// Static member initialization
lv_obj_t *ClubScreen::screen = nullptr;
std::vector<Club> ClubScreen::clubs;
std::function<void(int, const String &)> ClubScreen::selectedCallback;
SnapCallApi *ClubScreen::apiInstance = nullptr;
bool ClubScreen::isLoading = false;

void ClubScreen::show(SnapCallApi *api, std::function<void(int, const String &)> onClubSelected)
{
    Serial.printf("%s Showing club selection screen\n", TAG);

    apiInstance = api;
    selectedCallback = onClubSelected;

    // Create screen
    screen = lv_obj_create(NULL);
    lv_scr_load(screen);
    lv_obj_set_size(screen, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x000000), LV_PART_MAIN);

    // Title
    lv_obj_t *titleLabel = lv_label_create(screen);
    lv_label_set_text(titleLabel, "SNAPCALL");
    lv_obj_set_style_text_font(titleLabel, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(titleLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(titleLabel, LV_ALIGN_TOP_MID, 0, 10);

    // Subtitle
    lv_obj_t *subtitleLabel = lv_label_create(screen);
    lv_label_set_text(subtitleLabel, "SELECT CLUB");
    lv_obj_set_style_text_font(subtitleLabel, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(subtitleLabel, lv_color_hex(0xCCCCCC), LV_PART_MAIN);
    lv_obj_align(subtitleLabel, LV_ALIGN_TOP_MID, 0, 40);

    // Container for club list
    lv_obj_t *container = lv_obj_create(screen);
    lv_obj_set_size(container, 280, 180);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x1a1a1a), LV_PART_MAIN);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(container, LV_DIR_VER);

    // Loading label
    lv_obj_t *loadingLabel = lv_label_create(container);
    lv_label_set_text(loadingLabel, "Loading clubs...");
    lv_obj_set_style_text_color(loadingLabel, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(loadingLabel, LV_ALIGN_CENTER, 0, 0);

    isLoading = true;
    loadClubs();
}

void ClubScreen::hide()
{
    if (screen != nullptr)
    {
        lv_obj_del(screen);
        screen = nullptr;
    }
}

void ClubScreen::loadClubs()
{
    Serial.printf("%s Loading clubs from API\n", TAG);

    if (apiInstance == nullptr)
    {
        showError("API not initialized");
        return;
    }

    clubs = apiInstance->getClubs();
    isLoading = false;

    if (clubs.empty())
    {
        showError("No clubs found");
        return;
    }

    // Clear and recreate the container with club buttons
    if (screen == nullptr)
        return;

    // Find container
    lv_obj_t *container = NULL;
    lv_obj_t *child = lv_obj_get_child(screen, NULL);
    while (child != NULL)
    {
        if (lv_obj_get_height(child) == 180)
        { // Container height
            container = child;
            break;
        }
        child = lv_obj_get_child(screen, child);
    }

    if (container == NULL)
        return;

    // Clear container
    lv_obj_clean(container);

    // Add club buttons
    for (const Club &club : clubs)
    {
        lv_obj_t *btn = lv_btn_create(container);
        lv_obj_set_width(btn, lv_pct(100));
        lv_obj_set_height(btn, 40);
        lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(btn, lv_color_hex(0x3a3a3a), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0a3a7a), LV_PART_MAIN);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x0a5aaa), LV_STATE_PRESSED);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, club.name.c_str());
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
        lv_obj_center(label);

        // Store club ID in user data
        btn->user_data = (void *)(intptr_t)club.id;

        lv_obj_add_event_cb(btn, onClubClicked, LV_EVENT_CLICKED, NULL);
    }

    Serial.printf("%s Club screen loaded with %d clubs\n", TAG, clubs.size());
}

void ClubScreen::onClubClicked(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    int clubId = (intptr_t)btn->user_data;

    // Find club name
    String clubName = "";
    for (const Club &club : clubs)
    {
        if (club.id == clubId)
        {
            clubName = club.name;
            break;
        }
    }

    Serial.printf("%s Club selected: %s (ID: %d)\n", TAG, clubName.c_str(), clubId);

    if (selectedCallback)
    {
        selectedCallback(clubId, clubName);
    }
}

void ClubScreen::showError(const String &message)
{
    Serial.printf("%s Error: %s\n", TAG, message.c_str());

    if (screen == nullptr)
        return;

    // Find and clear container
    lv_obj_t *child = lv_obj_get_child(screen, NULL);
    while (child != NULL)
    {
        if (lv_obj_get_height(child) == 180)
        {
            lv_obj_clean(child);

            // Add error message
            lv_obj_t *errorLabel = lv_label_create(child);
            lv_label_set_text(errorLabel, message.c_str());
            lv_obj_set_style_text_color(errorLabel, lv_color_hex(0xFF6666), LV_PART_MAIN);
            lv_obj_align(errorLabel, LV_ALIGN_CENTER, 0, -20);

            // Add retry button
            lv_obj_t *retryBtn = lv_btn_create(child);
            lv_obj_set_size(retryBtn, 100, 40);
            lv_obj_align(retryBtn, LV_ALIGN_CENTER, 0, 20);
            lv_obj_set_style_bg_color(retryBtn, lv_color_hex(0x3a3a3a), LV_PART_MAIN);

            lv_obj_t *retryLabel = lv_label_create(retryBtn);
            lv_label_set_text(retryLabel, "Retry");
            lv_obj_center(retryLabel);

            lv_obj_add_event_cb(retryBtn, [](lv_event_t *e)
                                {
                // Retry loading clubs
                ClubScreen::loadClubs(); }, LV_EVENT_CLICKED, NULL);

            break;
        }
        child = lv_obj_get_child(screen, child);
    }
}
