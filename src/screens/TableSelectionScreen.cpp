#include "TableSelectionScreen.h"

void TableSelectionScreen::show(const Tournament &tournament, SelectCallback onSelect)
{
    onSelect_ = std::move(onSelect);
    buttonContexts_.clear();

    lv_obj_t *title_label = lv_label_create(lv_scr_act());
    char title_buf[64];
    snprintf(title_buf, sizeof(title_buf), "%s - Select Table", tournament.name.c_str());
    lv_label_set_text(title_label, title_buf);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 40);

    int y_offset = 70;

    for (const auto &table : tournament.tables)
    {
        std::unique_ptr<ButtonContext> ctx(new ButtonContext{this, table});
        ButtonContext *ctxPtr = ctx.get();
        buttonContexts_.push_back(std::move(ctx));

        lv_obj_t *btn = lv_btn_create(lv_scr_act());
        lv_obj_set_size(btn, 200, 60);
        lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x222222), 0);
        lv_obj_set_style_border_color(btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(btn, 1, 0);

        lv_obj_add_event_cb(btn, buttonEventCb, LV_EVENT_CLICKED, ctxPtr);

        lv_obj_t *btn_label = lv_label_create(btn);
        char btn_text[32];
        snprintf(btn_text, sizeof(btn_text), "Table %d", table.tableNumber);
        lv_label_set_text(btn_label, btn_text);
        lv_obj_set_style_text_color(btn_label, lv_color_white(), 0);
        lv_obj_center(btn_label);

        y_offset += 70;
    }
}

void TableSelectionScreen::buttonEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *ctx = static_cast<ButtonContext *>(lv_event_get_user_data(e));
    if (ctx && ctx->screen && ctx->screen->onSelect_)
    {
        ctx->screen->onSelect_(ctx->table);
    }
}
