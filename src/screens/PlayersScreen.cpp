#include "PlayersScreen.h"

namespace
{
    constexpr int kColumns = 2;
    constexpr int kMaxRows = 5;
    constexpr int kLeftMargin = 5;
    constexpr int kColumnWidth = 110;
    constexpr int kColumnGap = 10;
    constexpr int kGridTop = 50;
    constexpr int kRowHeight = 44;
    constexpr int kRowGap = 6;
}

void PlayersScreen::show(const std::vector<Player> &players, std::function<void()> onNavigateToBlinds, SelectCallback onSelect)
{
    onSelect_ = std::move(onSelect);
    buttonContexts_.clear();

    navBar_.show(NavTarget::Players, std::move(onNavigateToBlinds), std::function<void()>());

    size_t maxSeats = static_cast<size_t>(kColumns * kMaxRows);

    for (size_t i = 0; i < players.size() && i < maxSeats; i++)
    {
        const Player &player = players[i];

        int col = static_cast<int>(i) % kColumns;
        int row = static_cast<int>(i) / kColumns;

        int x = kLeftMargin + col * (kColumnWidth + kColumnGap);
        int y = kGridTop + row * (kRowHeight + kRowGap);

        lv_obj_t *btn = lv_btn_create(lv_scr_act());
        lv_obj_set_size(btn, kColumnWidth, kRowHeight);
        lv_obj_set_pos(btn, x, y);
        lv_obj_set_style_bg_color(btn, lv_color_hex(0x222222), 0);
        lv_obj_set_style_border_color(btn, lv_color_white(), 0);
        lv_obj_set_style_border_width(btn, 1, 0);

        if (player.firstName.length() > 0)
        {
            std::unique_ptr<ButtonContext> ctx(new ButtonContext{this, player});
            ButtonContext *ctxPtr = ctx.get();
            buttonContexts_.push_back(std::move(ctx));
            lv_obj_add_event_cb(btn, buttonEventCb, LV_EVENT_CLICKED, ctxPtr);
        }

        lv_obj_t *label = lv_label_create(btn);
        lv_label_set_text(label, formatPlayerLabel(player).c_str());
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_center(label);
    }
}

void PlayersScreen::buttonEventCb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    auto *ctx = static_cast<ButtonContext *>(lv_event_get_user_data(e));
    if (ctx && ctx->screen && ctx->screen->onSelect_)
    {
        ctx->screen->onSelect_(ctx->player);
    }
}
