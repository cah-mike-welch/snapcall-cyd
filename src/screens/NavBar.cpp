#include "NavBar.h"

namespace
{
    constexpr int kButtonWidth = 100;
    constexpr int kButtonHeight = 34;
    constexpr int kTopMargin = 5;
    constexpr int kSideMargin = 15;
    constexpr int kButtonGap = 10;
    constexpr uint32_t kActiveBgColor = 0x3355AA;
    constexpr uint32_t kInactiveBgColor = 0x222222;
}

void NavBar::show(NavTarget active, std::function<void()> onBlinds, std::function<void()> onPlayers)
{
    onBlinds_ = std::move(onBlinds);
    onPlayers_ = std::move(onPlayers);

    lv_obj_t *blindsBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(blindsBtn, kButtonWidth, kButtonHeight);
    lv_obj_set_pos(blindsBtn, kSideMargin, kTopMargin);
    lv_obj_set_style_bg_color(blindsBtn, lv_color_hex(active == NavTarget::Blinds ? kActiveBgColor : kInactiveBgColor), 0);
    lv_obj_set_style_border_color(blindsBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(blindsBtn, 1, 0);
    lv_obj_add_event_cb(blindsBtn, blindsBtnEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *blindsLabel = lv_label_create(blindsBtn);
    lv_label_set_text(blindsLabel, "Blinds");
    lv_obj_set_style_text_color(blindsLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(blindsLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(blindsLabel);

    lv_obj_t *playersBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(playersBtn, kButtonWidth, kButtonHeight);
    lv_obj_set_pos(playersBtn, kSideMargin + kButtonWidth + kButtonGap, kTopMargin);
    lv_obj_set_style_bg_color(playersBtn, lv_color_hex(active == NavTarget::Players ? kActiveBgColor : kInactiveBgColor), 0);
    lv_obj_set_style_border_color(playersBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(playersBtn, 1, 0);
    lv_obj_add_event_cb(playersBtn, playersBtnEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *playersLabel = lv_label_create(playersBtn);
    lv_label_set_text(playersLabel, "Players");
    lv_obj_set_style_text_color(playersLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(playersLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(playersLabel);
}

void NavBar::blindsBtnEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *nav = static_cast<NavBar *>(lv_event_get_user_data(e));
    if (nav && nav->onBlinds_)
    {
        nav->onBlinds_();
    }
}

void NavBar::playersBtnEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *nav = static_cast<NavBar *>(lv_event_get_user_data(e));
    if (nav && nav->onPlayers_)
    {
        nav->onPlayers_();
    }
}
