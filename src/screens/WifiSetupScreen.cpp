#include "WifiSetupScreen.h"
#include "UiTheme.h"

namespace
{
    constexpr lv_coord_t kListWidth = 220;
    constexpr lv_coord_t kListHeight = 220;
    constexpr lv_coord_t kRowWidth = 200;
    constexpr lv_coord_t kRowHeight = 40;
    constexpr lv_coord_t kRowSpacing = 46;

    lv_obj_t *createButton(lv_obj_t *parent, const char *text, lv_coord_t width, lv_coord_t height, const lv_font_t *font)
    {
        lv_obj_t *btn = lv_btn_create(parent);
        lv_obj_set_size(btn, width, height);
        UiTheme::styleButton(btn);
        lv_obj_set_style_border_color(btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(btn, 1, 0);

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, text);
        lv_label_set_long_mode(label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(label, width - 12);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, font, 0);
        lv_obj_center(label);
        return btn;
    }

    lv_obj_t *createWhiteLabel(lv_obj_t *parent, const char *text, const lv_font_t *font)
    {
        lv_obj_t *label = lv_label_create(parent);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, font, 0);
        return label;
    }
}

void WifiSetupScreen::clear()
{
    lv_obj_clean(lv_scr_act());
    ssids_.clear();
    statusLabel_ = nullptr;
    passwordField_ = nullptr;
}

void WifiSetupScreen::showNoWifi(std::function<void()> onSetup)
{
    onSetup_ = std::move(onSetup);
    clear();

    lv_obj_t *title = createWhiteLabel(lv_scr_act(), "No WiFi", &lv_font_montserrat_28);
    lv_obj_align(title, LV_ALIGN_CENTER, 0, -80);

    lv_obj_t *message = createWhiteLabel(lv_scr_act(), "Could not connect to a saved network.", &lv_font_montserrat_14);
    lv_label_set_long_mode(message, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(message, 210);
    lv_obj_set_style_text_align(message, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(message, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *setupBtn = createButton(lv_scr_act(), "Setup WiFi", 200, 60, &lv_font_montserrat_20);
    lv_obj_align(setupBtn, LV_ALIGN_CENTER, 0, 40);
    lv_obj_add_event_cb(setupBtn, actionEventCb, LV_EVENT_CLICKED, &onSetup_);
}

void WifiSetupScreen::showMessage(const String &text)
{
    clear();

    lv_obj_t *label = createWhiteLabel(lv_scr_act(), text.c_str(), &lv_font_montserrat_20);
    lv_obj_center(label);
}

void WifiSetupScreen::showNetworkList(const std::vector<WifiScanResult> &networks,
                                      std::function<void(const String &)> onSelect,
                                      std::function<void()> onRescan,
                                      std::function<void()> onBack)
{
    onSelect_ = std::move(onSelect);
    onRescan_ = std::move(onRescan);
    onBack_ = std::move(onBack);
    clear();

    lv_obj_t *title = createWhiteLabel(lv_scr_act(), "Select WiFi", &lv_font_montserrat_20);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Scrollable list so every scanned network stays reachable on the 240 px wide panel.
    lv_obj_t *list = lv_obj_create(lv_scr_act());
    lv_obj_set_size(list, kListWidth, kListHeight);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 44);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);

    if (networks.empty())
    {
        lv_obj_t *none = createWhiteLabel(list, "No networks found.\nTap Rescan to try again.", &lv_font_montserrat_14);
        lv_obj_set_style_text_align(none, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(none, LV_ALIGN_TOP_MID, 0, 40);
    }

    lv_coord_t y = 0;
    for (const auto &network : networks)
    {
        ssids_.push_back(network.ssid);
        lv_obj_t *row = createButton(list, network.ssid.c_str(), kRowWidth, kRowHeight, &lv_font_montserrat_14);
        lv_obj_align(row, LV_ALIGN_TOP_MID, 0, y);
        lv_obj_add_event_cb(row, networkRowEventCb, LV_EVENT_CLICKED, this);
        y += kRowSpacing;
    }

    lv_obj_t *rescanBtn = createButton(lv_scr_act(), "Rescan", 100, 40, &lv_font_montserrat_14);
    lv_obj_align(rescanBtn, LV_ALIGN_BOTTOM_MID, -60, -10);
    lv_obj_add_event_cb(rescanBtn, actionEventCb, LV_EVENT_CLICKED, &onRescan_);

    lv_obj_t *backBtn = createButton(lv_scr_act(), "Back", 100, 40, &lv_font_montserrat_14);
    lv_obj_align(backBtn, LV_ALIGN_BOTTOM_MID, 60, -10);
    lv_obj_add_event_cb(backBtn, actionEventCb, LV_EVENT_CLICKED, &onBack_);
}

void WifiSetupScreen::showPasswordEntry(const String &ssid,
                                        std::function<void(const String &)> onConnect,
                                        std::function<void()> onCancel)
{
    onConnect_ = std::move(onConnect);
    onCancel_ = std::move(onCancel);
    clear();

    statusLabel_ = createWhiteLabel(lv_scr_act(), ("Password for " + ssid).c_str(), &lv_font_montserrat_14);
    lv_label_set_long_mode(statusLabel_, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(statusLabel_, 220);
    lv_obj_set_style_text_align(statusLabel_, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(statusLabel_, LV_ALIGN_TOP_MID, 0, 6);

    passwordField_ = lv_textarea_create(lv_scr_act());
    lv_textarea_set_one_line(passwordField_, true);
    lv_textarea_set_password_mode(passwordField_, true);
    lv_textarea_set_placeholder_text(passwordField_, "Password");
    lv_obj_set_size(passwordField_, 220, 36);
    lv_obj_set_style_text_font(passwordField_, &lv_font_montserrat_14, 0);
    lv_obj_align(passwordField_, LV_ALIGN_TOP_MID, 0, 44);

    lv_obj_t *cancelBtn = createButton(lv_scr_act(), "Cancel", 100, 36, &lv_font_montserrat_14);
    lv_obj_align(cancelBtn, LV_ALIGN_TOP_MID, -60, 88);
    lv_obj_add_event_cb(cancelBtn, actionEventCb, LV_EVENT_CLICKED, &onCancel_);

    lv_obj_t *connectBtn = createButton(lv_scr_act(), "Connect", 100, 36, &lv_font_montserrat_14);
    lv_obj_align(connectBtn, LV_ALIGN_TOP_MID, 60, 88);
    lv_obj_add_event_cb(connectBtn, connectEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *keyboard = lv_keyboard_create(lv_scr_act());
    lv_obj_set_size(keyboard, 240, 180);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_text_font(keyboard, &lv_font_montserrat_14, LV_PART_ITEMS);
    lv_keyboard_set_textarea(keyboard, passwordField_);
    lv_obj_add_event_cb(keyboard, connectEventCb, LV_EVENT_READY, this);
}

void WifiSetupScreen::setStatus(const String &text)
{
    if (statusLabel_)
    {
        lv_label_set_text(statusLabel_, text.c_str());
    }
}

void WifiSetupScreen::networkRowEventCb(lv_event_t *e)
{
    auto *screen = static_cast<WifiSetupScreen *>(lv_event_get_user_data(e));
    if (!screen)
    {
        return;
    }

    const uint32_t index = lv_obj_get_index(lv_event_get_target(e));
    if (index >= screen->ssids_.size())
    {
        return;
    }

    // Copy before invoking: the callback rebuilds this screen.
    const String ssid = screen->ssids_[index];
    auto onSelect = screen->onSelect_;
    if (onSelect)
    {
        onSelect(ssid);
    }
}

void WifiSetupScreen::actionEventCb(lv_event_t *e)
{
    auto *callback = static_cast<std::function<void()> *>(lv_event_get_user_data(e));
    if (!callback)
    {
        return;
    }

    // Copy before invoking: the callback rebuilds this screen and may replace the stored function.
    auto action = *callback;
    if (action)
    {
        action();
    }
}

// The connect work is deferred because it can delete the keyboard, which LVGL is still using in its READY handler.
void WifiSetupScreen::connectEventCb(lv_event_t *e)
{
    lv_async_call(runConnectAsync, lv_event_get_user_data(e));
}

void WifiSetupScreen::runConnectAsync(void *userData)
{
    auto *screen = static_cast<WifiSetupScreen *>(userData);
    if (!screen || !screen->passwordField_)
    {
        return;
    }

    const String password = lv_textarea_get_text(screen->passwordField_);
    auto onConnect = screen->onConnect_;
    if (onConnect)
    {
        onConnect(password);
    }
}
