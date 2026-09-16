#include "DisplayManager.h"

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "esp_timer.h"
#include "config/AppConfig.h"

namespace
{
    TFT_eSPI s_tft;
    lv_disp_draw_buf_t s_drawBuf;
    lv_color_t s_buf[AppConfig::kScreenWidth * 10];
    lv_disp_drv_t s_dispDrv;
}

void DisplayManager::tickCallback(void *arg)
{
    lv_tick_inc(1);
}

void DisplayManager::flushCallback(lv_disp_drv_t *dispDrv, const lv_area_t *area, lv_color_t *colorP)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    s_tft.startWrite();
    s_tft.setAddrWindow(area->x1, area->y1, w, h);
    s_tft.pushColors((uint16_t *)&colorP->full, w * h, true);
    s_tft.endWrite();
    lv_disp_flush_ready(dispDrv);
}

void DisplayManager::begin()
{
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    s_tft.init();
    s_tft.setRotation(0);

    lv_init();

    const esp_timer_create_args_t lvglTickTimerArgs = {
        .callback = &DisplayManager::tickCallback,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvglTickTimer = NULL;
    esp_timer_create(&lvglTickTimerArgs, &lvglTickTimer);
    esp_timer_start_periodic(lvglTickTimer, 1000);

    lv_disp_draw_buf_init(&s_drawBuf, s_buf, NULL, AppConfig::kScreenWidth * 10);
    lv_disp_drv_init(&s_dispDrv);
    s_dispDrv.hor_res = AppConfig::kScreenWidth;
    s_dispDrv.ver_res = AppConfig::kScreenHeight;
    s_dispDrv.flush_cb = &DisplayManager::flushCallback;
    s_dispDrv.draw_buf = &s_drawBuf;
    lv_disp_drv_register(&s_dispDrv);
}

void DisplayManager::setDarkBackground()
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);
}
