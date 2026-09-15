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
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable required widgets */
#define LV_USE_BTN 1
#define LV_USE_LABEL 1

#endif /* LV_CONF_H */