#pragma once

#include <lvgl.h>

// Owns the TFT + LVGL display pipeline: init, tick timer, draw buffer, driver, flush callback.
class DisplayManager
{
public:
    void begin();
    void setDarkBackground();

private:
    static void flushCallback(lv_disp_drv_t *dispDrv, const lv_area_t *area, lv_color_t *colorP);
    static void tickCallback(void *arg);
};
