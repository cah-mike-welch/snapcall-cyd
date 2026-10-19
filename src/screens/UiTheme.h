#pragma once

#include <lvgl.h>

namespace UiTheme
{
    // The panel renders the emerald-600 family as teal, so the button green is
    // #008000 (chosen from an on-device swatch test).
    constexpr uint32_t kButtonBgColor = 0x008000;

    // Sets the button background for both normal and pressed states so the
    // LVGL default theme's pressed color doesn't show through.
    inline void styleButton(lv_obj_t *btn)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(kButtonBgColor), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(kButtonBgColor), LV_STATE_PRESSED);
    }
}
