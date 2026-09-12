#include "Display.h"
#include "config.h"
#include <XPT2046_Touchscreen.h>

static const char *TAG = "[DISPLAY]";

// TFT_eSPI instance
TFT_eSPI Display::tft;

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DISPLAY_WIDTH * 40]; // 40 lines of buffer

// Touch input
static XPT2046_Touchscreen ts(TOUCH_CS);
static TS_Point touchPoint;
static bool touchPressed = false;

// Forward declarations for LVGL callbacks
static void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);

void Display::begin()
{
    Serial.printf("%s Initializing display\n", TAG);

    // Initialize TFT_eSPI
    tft.init();
    Serial.printf("%s TFT initialized\n", TAG);

    // Set rotation to landscape
    tft.setRotation(1); // 0=portrait, 1=landscape
    tft.fillScreen(TFT_BLACK);

    // Initialize backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    Serial.printf("%s Backlight initialized\n", TAG);

    // Initialize LVGL
    lv_init();
    Serial.printf("%s LVGL initialized\n", TAG);

    // Setup LVGL display
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, DISPLAY_WIDTH * 40);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = DISPLAY_WIDTH;
    disp_drv.ver_res = DISPLAY_HEIGHT;
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = lvgl_flush_cb;
    lv_disp_drv_register(&disp_drv);

    // Setup LVGL touch input
    ts.begin();
    Serial.printf("%s Touchscreen initialized\n", TAG);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    lv_indev_drv_register(&indev_drv);

    Serial.printf("%s Display initialization complete\n", TAG);
}

void Display::update()
{
    lv_timer_handler();
}

TFT_eSPI &Display::getTFT()
{
    return tft;
}

void Display::clear()
{
    tft.fillScreen(TFT_BLACK);
    lv_obj_clean(lv_scr_act());
}

// Global callback wrappers (these call the static methods)
static void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    Display::getTFT().startWrite();
    Display::getTFT().setAddrWindow(area->x1, area->y1, w, h);
    Display::getTFT().pushColors((uint16_t *)color_p, w * h, true);
    Display::getTFT().endWrite();

    lv_disp_flush_ready(disp);
}

static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (ts.touched())
    {
        TS_Point p = ts.getPoint();
        data->point.x = p.x;
        data->point.y = p.y;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
