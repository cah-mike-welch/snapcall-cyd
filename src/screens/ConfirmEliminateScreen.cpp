#include "ConfirmEliminateScreen.h"

namespace
{
    constexpr int kButtonWidth = 100;
    constexpr int kButtonHeight = 50;
}

void ConfirmEliminateScreen::show(const Player &player, std::function<void()> onConfirm, std::function<void()> onCancel)
{
    onConfirm_ = std::move(onConfirm);
    onCancel_ = std::move(onCancel);

    lv_obj_t *title_label = lv_label_create(lv_scr_act());
    lv_label_set_text(title_label, "Eliminate Player?");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 60);

    char name_buf[64];
    snprintf(name_buf, sizeof(name_buf), "%s %s", player.firstName.c_str(), player.lastName.c_str());
    lv_obj_t *name_label = lv_label_create(lv_scr_act());
    lv_label_set_text(name_label, name_buf);
    lv_obj_set_style_text_color(name_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(name_label, &lv_font_montserrat_28, 0);
    lv_obj_align(name_label, LV_ALIGN_CENTER, 0, -20);

    lv_obj_t *eliminateBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(eliminateBtn, kButtonWidth, kButtonHeight);
    lv_obj_align(eliminateBtn, LV_ALIGN_BOTTOM_MID, -60, -40);
    lv_obj_set_style_bg_color(eliminateBtn, lv_color_hex(0x881111), 0);
    lv_obj_set_style_border_color(eliminateBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(eliminateBtn, 1, 0);
    lv_obj_add_event_cb(eliminateBtn, eliminateBtnEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *eliminateLabel = lv_label_create(eliminateBtn);
    lv_label_set_text(eliminateLabel, "Eliminate");
    lv_obj_set_style_text_color(eliminateLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(eliminateLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(eliminateLabel);

    lv_obj_t *cancelBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(cancelBtn, kButtonWidth, kButtonHeight);
    lv_obj_align(cancelBtn, LV_ALIGN_BOTTOM_MID, 60, -40);
    lv_obj_set_style_bg_color(cancelBtn, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_color(cancelBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(cancelBtn, 1, 0);
    lv_obj_add_event_cb(cancelBtn, cancelBtnEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *cancelLabel = lv_label_create(cancelBtn);
    lv_label_set_text(cancelLabel, "Cancel");
    lv_obj_set_style_text_color(cancelLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(cancelLabel, &lv_font_montserrat_16, 0);
    lv_obj_center(cancelLabel);
}

void ConfirmEliminateScreen::eliminateBtnEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *screen = static_cast<ConfirmEliminateScreen *>(lv_event_get_user_data(e));
    if (screen && screen->onConfirm_)
    {
        screen->onConfirm_();
    }
}

void ConfirmEliminateScreen::cancelBtnEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *screen = static_cast<ConfirmEliminateScreen *>(lv_event_get_user_data(e));
    if (screen && screen->onCancel_)
    {
        screen->onCancel_();
    }
}
