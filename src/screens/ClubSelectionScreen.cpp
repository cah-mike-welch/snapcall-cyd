#include "ClubSelectionScreen.h"
#include "UiTheme.h"
#include "assets/Logo.h"

void ClubSelectionScreen::show(const std::vector<Club> &clubs, SelectCallback onSelect)
{
    onSelect_ = std::move(onSelect);
    buttonContexts_.clear();

    lv_obj_t *logo = lv_img_create(lv_scr_act());
    lv_img_set_src(logo, &Assets::kLogo);
    lv_obj_align(logo, LV_ALIGN_TOP_MID, 0, 8);

    lv_obj_t *title_label = lv_label_create(lv_scr_act());
    lv_label_set_text(title_label, "Select a Club");
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 128);

    // Scrollable list so the club buttons still fit below the logo and title.
    lv_obj_t *list = lv_obj_create(lv_scr_act());
    lv_obj_set_size(list, 220, 152);
    lv_obj_align(list, LV_ALIGN_TOP_MID, 0, 164);
    lv_obj_set_style_bg_opa(list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    lv_obj_set_style_pad_all(list, 0, 0);

    int y_offset = 0;

    for (const auto &club : clubs)
    {
        std::unique_ptr<ButtonContext> ctx(new ButtonContext{this, club});
        ButtonContext *ctxPtr = ctx.get();
        buttonContexts_.push_back(std::move(ctx));

        lv_obj_t *btn = lv_btn_create(list);
        lv_obj_set_size(btn, 200, 40);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset);
        UiTheme::styleButton(btn);
        lv_obj_set_style_border_color(btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(btn, 1, 0);

        lv_obj_add_event_cb(btn, buttonEventCb, LV_EVENT_CLICKED, ctxPtr);

        lv_obj_t *btn_label = lv_label_create(btn);
        lv_label_set_text(btn_label, club.shortName.c_str());
        lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
        lv_obj_center(btn_label);

        y_offset += 50;
    }
}

void ClubSelectionScreen::buttonEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *ctx = static_cast<ButtonContext *>(lv_event_get_user_data(e));
    if (ctx && ctx->screen && ctx->screen->onSelect_)
    {
        ctx->screen->onSelect_(ctx->club);
    }
}
