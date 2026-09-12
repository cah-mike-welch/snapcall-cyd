#include "Display.h"
#include "config.h"

static const char *TAG = "[DISPLAY]";

// TFT_eSPI instance
TFT_eSPI Display::tft;

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DISPLAY_WIDTH * 40]; // 320 * 40 buffer

// LVGL input device
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev;

// Touch input - Placeholder for future XPT2046 support
// For now, using LVGL's built-in pointer input

// XPT2046 touch reading - use TFT_eSPI's built-in touch support
static uint16_t lastX = 0, lastY = 0;
static bool lastTouched = false;

static bool readTouch()
{
    uint16_t t_x = 0, t_y = 0;
    
    if (Display::getTFT().getTouch(&t_x, &t_y)) {
        lastX = t_x;
        lastY = t_y;
        lastTouched = true;
        return true;
    }
    return false;
}

// Forward declarations for LVGL callbacks
static void lvgl_flush_cb(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data);

void Display::begin()
{
    Serial.printf("%s Initializing display\n", TAG);

    // Initialize TFT_eSPI
    tft.init();
    Serial.printf("%s TFT initialized\n", TAG);

    // Set rotation for landscape (CYD: 1=landscape 320x240)
    tft.setRotation(1);  // 90 degrees for standard CYD landscape
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
    disp_drv.hor_res = DISPLAY_WIDTH;   // 320
    disp_drv.ver_res = DISPLAY_HEIGHT;  // 240
    disp_drv.draw_buf = &draw_buf;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.rotated = LV_DISP_ROT_270;  // 270-degree rotation for landscape CYD
    lv_disp_drv_register(&disp_drv);

    // Set default theme and font after display is registered
    lv_theme_t * th = lv_theme_default_init(lv_disp_get_default(), lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), true, LV_FONT_DEFAULT);
    lv_disp_set_theme(lv_disp_get_default(), th);

    // Setup LVGL input device for touchscreen (basic support)
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    indev = lv_indev_drv_register(&indev_drv);

    Serial.printf("%s Touchscreen input setup complete\n", TAG);

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

    // Debug: Log flush area every 10th time to reduce spam
    static uint32_t flush_count = 0;
    if (flush_count++ % 10 == 0) {
        Serial.printf("[FLUSH] area: (%d,%d)-(%d,%d) %dx%d\n", 
            area->x1, area->y1, area->x2, area->y2, w, h);
    }

    Display::getTFT().startWrite();
    Display::getTFT().setAddrWindow(area->x1, area->y1, w, h);
    Display::getTFT().pushColors((uint16_t *)color_p, w * h, true);
    Display::getTFT().endWrite();

    lv_disp_flush_ready(disp);
}

static void lvgl_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (readTouch()) {
        // TFT_eSPI returns coordinates in screen space, but need to account for LVGL rotation
        // For ROT_270, swap coordinates
        data->point.x = lastY;  // Swapped due to rotation
        data->point.y = lastX;  // Swapped due to rotation
        data->state = LV_INDEV_STATE_PRESSED;
        
        static uint32_t touch_count = 0;
        if (touch_count++ % 5 == 0) {
            Serial.printf("[TOUCH] Screen: X=%d Y=%d -> LVGL: X=%d Y=%d\n", 
                lastX, lastY, data->point.x, data->point.y);
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
