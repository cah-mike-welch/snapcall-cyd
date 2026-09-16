#include "TouchManager.h"

#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include "config/AppConfig.h"

namespace
{
    SPIClass s_touchSpi(HSPI);
    XPT2046_Touchscreen s_ts(AppConfig::kTouchCsPin, AppConfig::kTouchIrqPin);
    lv_indev_drv_t s_indevDrv;
}

void TouchManager::readCallback(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (s_ts.touched())
    {
        TS_Point p = s_ts.getPoint();
        int16_t x = map(p.x, AppConfig::kTouchMinX, AppConfig::kTouchMaxX, 0, AppConfig::kScreenWidth);
        int16_t y = map(p.y, AppConfig::kTouchMinY, AppConfig::kTouchMaxY, 0, AppConfig::kScreenHeight);
        x = constrain(x, 0, AppConfig::kScreenWidth - 1);
        y = constrain(y, 0, AppConfig::kScreenHeight - 1);

        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

void TouchManager::begin()
{
    pinMode(AppConfig::kTouchIrqPin, INPUT_PULLUP);

    s_touchSpi.begin(AppConfig::kTouchClkPin, AppConfig::kTouchMisoPin, AppConfig::kTouchMosiPin, AppConfig::kTouchCsPin);
    s_ts.begin(s_touchSpi);
    s_ts.setRotation(0);

    lv_indev_drv_init(&s_indevDrv);
    s_indevDrv.type = LV_INDEV_TYPE_POINTER;
    s_indevDrv.read_cb = &TouchManager::readCallback;
    lv_indev_drv_register(&s_indevDrv);
}
