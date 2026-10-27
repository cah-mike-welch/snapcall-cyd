#pragma once

#include <lvgl.h>

namespace UiTheme
{
    // The panel renders the emerald-600 family as teal, so the button green is
    // #008000 (chosen from an on-device swatch test).
    constexpr uint32_t kButtonBgColor = 0x008000;

    // Pressed feedback: very dark green fill with a thicker, darker border.
    constexpr uint32_t kButtonPressedBgColor = 0x003300;
    constexpr uint32_t kButtonPressedBorderColor = 0x333333;
    constexpr int kButtonPressedBorderWidth = 3;

    // Sets the normal and pressed styles. Pressed overrides the LVGL default
    // theme's darken so the press is clearly visible.
    inline void styleButton(lv_obj_t *btn)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(kButtonBgColor), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(kButtonPressedBgColor), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(btn, lv_color_hex(kButtonPressedBorderColor), LV_STATE_PRESSED);
        lv_obj_set_style_border_width(btn, kButtonPressedBorderWidth, LV_STATE_PRESSED);
    }

    // Gray background and dark gray border for empty (open) seats.
    constexpr uint32_t kOpenSeatBgColor = 0x666666;
    constexpr uint32_t kOpenSeatBorderColor = 0x333333;

    inline void styleOpenSeat(lv_obj_t *btn)
    {
        lv_obj_set_style_bg_color(btn, lv_color_hex(kOpenSeatBgColor), 0);
        lv_obj_set_style_bg_color(btn, lv_color_hex(kOpenSeatBgColor), LV_STATE_PRESSED);
        lv_obj_set_style_border_color(btn, lv_color_hex(kOpenSeatBorderColor), 0);
    }
}
