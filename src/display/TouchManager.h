#pragma once

#include <lvgl.h>

// Owns the XPT2046 touchscreen hardware and its LVGL input driver.
class TouchManager
{
public:
    void begin();

private:
    static void readCallback(lv_indev_drv_t *drv, lv_indev_data_t *data);
};
