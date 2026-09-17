#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

/* Enable LVGL configuration */
#define LV_CONF_SKIP 0

/* Color depth: 16 (RGB565) */
#define LV_COLOR_16_SWAP 1
#define LV_COLOR_DEPTH 16

/* Memory settings */
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (32U * 1024U) /* 32kB RAM allocation */

/* HAL Settings */
#define LV_DISP_DEF_REFR_PERIOD 30  /* Refresh rate ~33 FPS */
#define LV_INDEV_DEF_READ_PERIOD 30 /* Input polling rate */

/* Feature usage */
#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

/* Enable default font */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_28

/* Enable required widgets */
#define LV_USE_BTN 1
#define LV_USE_LABEL 1

/* Disable unused basic widgets to save flash (only lv_btn/lv_label are used) */
#define LV_USE_ARC 0
#define LV_USE_BAR 0
#define LV_USE_BTNMATRIX 0
#define LV_USE_CANVAS 0
#define LV_USE_CHECKBOX 0
#define LV_USE_DROPDOWN 0
#define LV_USE_IMG 0
#define LV_USE_LINE 0
#define LV_USE_ROLLER 0
#define LV_USE_SLIDER 0
#define LV_USE_SWITCH 0
#define LV_USE_TABLE 0
#define LV_USE_TEXTAREA 0

/* Disable unused extra widgets to save flash */
#define LV_USE_ANIMIMG 0
#define LV_USE_CALENDAR 0
#define LV_USE_CHART 0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN 0
#define LV_USE_KEYBOARD 0
#define LV_USE_LED 0
#define LV_USE_LIST 0
#define LV_USE_MENU 0
#define LV_USE_METER 0
#define LV_USE_MSGBOX 0
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 0
#define LV_USE_TABVIEW 0
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

/* Disable unused layouts and misc extras (no flex/grid/fragment/etc. calls anywhere) */
#define LV_USE_FLEX 0
#define LV_USE_GRID 0
#define LV_USE_FRAGMENT 0
#define LV_USE_GRIDNAV 0
#define LV_USE_IME_PINYIN 0
#define LV_USE_IMGFONT 0
#define LV_USE_MONKEY 0
#define LV_USE_MSG 0
#define LV_USE_SNAPSHOT 0

#endif /* LV_CONF_H */