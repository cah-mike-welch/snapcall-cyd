#include "ConfirmClockScreen.h"
#include "UiTheme.h"

namespace
{
    constexpr int kBigButtonWidth = 200;
    constexpr int kBigButtonHeight = 90;
    constexpr int kCancelButtonWidth = 100;
    constexpr int kCancelButtonHeight = 50;
}

void ConfirmClockScreen::show(bool isRunning, std::function<void()> onConfirm, std::function<void()> onCancel)
{
    onConfirm_ = std::move(onConfirm);
    onCancel_ = std::move(onCancel);

    lv_obj_t *prompt_label = lv_label_create(lv_scr_act());
    lv_label_set_text(prompt_label, isRunning ? "Stop Clock?" : "OK to start the clock?");
    lv_obj_set_width(prompt_label, 220);
    lv_obj_set_style_text_align(prompt_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(prompt_label, lv_color_white(), 0);
    lv_obj_set_style_text_font(prompt_label, &lv_font_montserrat_20, 0);
    lv_obj_align(prompt_label, LV_ALIGN_CENTER, 0, -80);

    lv_obj_t *confirmBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(confirmBtn, kBigButtonWidth, kBigButtonHeight);
    lv_obj_align(confirmBtn, LV_ALIGN_CENTER, 0, -10);
    UiTheme::styleButton(confirmBtn);
    lv_obj_set_style_border_color(confirmBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(confirmBtn, 1, 0);
    lv_obj_add_event_cb(confirmBtn, confirmBtnEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *confirmLabel = lv_label_create(confirmBtn);
    lv_label_set_text(confirmLabel, isRunning ? "Stop Clock" : "Start Clock");
    lv_obj_set_style_text_color(confirmLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(confirmLabel, &lv_font_montserrat_28, 0);
    lv_obj_center(confirmLabel);

    lv_obj_t *cancelBtn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(cancelBtn, kCancelButtonWidth, kCancelButtonHeight);
    lv_obj_align(cancelBtn, LV_ALIGN_BOTTOM_MID, 0, -40);
    UiTheme::styleButton(cancelBtn);
    lv_obj_set_style_border_color(cancelBtn, lv_color_white(), 0);
    lv_obj_set_style_border_width(cancelBtn, 1, 0);
    lv_obj_add_event_cb(cancelBtn, cancelBtnEventCb, LV_EVENT_CLICKED, this);

    lv_obj_t *cancelLabel = lv_label_create(cancelBtn);
    lv_label_set_text(cancelLabel, "Cancel");
    lv_obj_set_style_text_color(cancelLabel, lv_color_white(), 0);
    lv_obj_set_style_text_font(cancelLabel, &lv_font_montserrat_20, 0);
    lv_obj_center(cancelLabel);
}

void ConfirmClockScreen::confirmBtnEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *screen = static_cast<ConfirmClockScreen *>(lv_event_get_user_data(e));
    if (screen && screen->onConfirm_)
    {
        screen->onConfirm_();
    }
}

void ConfirmClockScreen::cancelBtnEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *screen = static_cast<ConfirmClockScreen *>(lv_event_get_user_data(e));
    if (screen && screen->onCancel_)
    {
        screen->onCancel_();
    }
}
