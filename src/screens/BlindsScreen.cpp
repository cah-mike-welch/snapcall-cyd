#include "BlindsScreen.h"

#include <lvgl.h>

void BlindsScreen::show(const Blinds &blinds, int tableNumber, std::function<void()> onNavigateToPlayers)
{
    timeLabel_ = nullptr;

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
    lv_obj_align(timeLabel_, LV_ALIGN_BOTTOM_MID, 0, -40);
}

void BlindsScreen::updateTimeRemaining(const String &timeRemaining)
{
    if (timeLabel_)
    {
        lv_label_set_text(timeLabel_, timeRemaining.c_str());
    }
}
