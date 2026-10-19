#include "BlindsScreen.h"

#include <lvgl.h>

#include "UiTheme.h"

namespace
{
    constexpr int kClockBtnWidth = 160;
    constexpr int kClockBtnHeight = 40;
    constexpr int kClockBtnBottomMargin = 12;
}

void BlindsScreen::show(const Blinds &blinds, int tableNumber, std::function<void()> onNavigateToPlayers, std::function<void()> onToggleClock)
{
    timeLabel_ = nullptr;
    onToggleClock_ = std::move(onToggleClock);

    navBar_.show(NavTarget::Blinds, std::function<void()>(), std::move(onNavigateToPlayers));

    // 1. Title Label (Table Number)
    lv_obj_t *title_label = lv_label_create(lv_scr_act());
    char title_buf[64];
    snprintf(title_buf, sizeof(title_buf), "Table %d Blinds", tableNumber);
    lv_label_set_text(title_label, title_buf);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 45);

    // 2. Main Blinds String (e.g., 800-1.6K)
    String blinds_text = blinds.smallBlind + "-" + blinds.bigBlind;
    lv_obj_t *blinds_label = lv_label_create(lv_scr_act());
    lv_label_set_text(blinds_label, blinds_text.c_str());
    lv_obj_set_style_text_color(blinds_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(blinds_label, &lv_font_montserrat_48, 0);

    // 3. Conditional Layout based on Big Blind Ante
    if (blinds.bigBlindAnte.toFloat() > 0)
    {
        lv_obj_align(blinds_label, LV_ALIGN_TOP_MID, 0, 80);

        // Ante line below blinds using smaller font (montserrat_28)
        String ante_text = blinds.bigBlindAnte + " BB ante";
        lv_obj_t *ante_label = lv_label_create(lv_scr_act());
        lv_label_set_text(ante_label, ante_text.c_str());
        lv_obj_set_style_text_color(ante_label, lv_color_white(), 0);
        lv_obj_set_style_text_font(ante_label, &lv_font_montserrat_28, 0);
        lv_obj_align(ante_label, LV_ALIGN_TOP_MID, 0, 145);
    }
    else
    {
        lv_obj_align(blinds_label, LV_ALIGN_CENTER, 0, -20);
    }

    // 4. Time Remaining String (Large font near bottom)
    timeLabel_ = lv_label_create(lv_scr_act());
    lv_label_set_text(timeLabel_, blinds.timeRemaining.c_str());
    lv_obj_set_style_text_color(timeLabel_, lv_color_white(), 0);
    lv_obj_set_style_text_font(timeLabel_, &lv_font_montserrat_48, 0);
    lv_obj_align(timeLabel_, LV_ALIGN_BOTTOM_MID, 0, -64);

    // 5. Clock start/stop button below the time remaining
    lv_obj_t *clockBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(clockBtn, kClockBtnWidth, kClockBtnHeight);
    lv_obj_align(clockBtn, LV_ALIGN_BOTTOM_MID, 0, -kClockBtnBottomMargin);
    UiTheme::styleButton(clockBtn);
    lv_obj_set_style_border_color(clockBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(clockBtn, 1, 0);
    lv_obj_add_event_cb(clockBtn, clockBtnEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *clockBtnLabel = lv_label_create(clockBtn);
    lv_label_set_text(clockBtnLabel, blinds.isRunning ? "Stop clock" : "Start clock");
    lv_obj_set_style_text_color(clockBtnLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(clockBtnLabel, &lv_font_montserrat_20, 0);
    lv_obj_center(clockBtnLabel);
}

void BlindsScreen::clockBtnEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *screen = static_cast<BlindsScreen *>(lv_event_get_user_data(e));
    if (screen && screen->onToggleClock_)
    {
        screen->onToggleClock_();
    }
}

void BlindsScreen::updateTimeRemaining(const String &timeRemaining)
{
    if (timeLabel_)
    {
        lv_label_set_text(timeLabel_, timeRemaining.c_str());
    }
}
