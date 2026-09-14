#include "Display.h"
#include "config.h"
#include <XPT2046_Touchscreen.h>
#include <SPI.h>

static const char *TAG = "[DISPLAY]";

// XPT2046 Touchscreen instance
// Using pins defined in User_Setup.h: CS=33, IRQ=36
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ);

// Calibration values for ESP32-2432S028
#define TOUCH_X_MIN 200
#define TOUCH_X_MAX 3900
#define TOUCH_Y_MIN 200
#define TOUCH_Y_MAX 3900

// TFT_eSPI instance
TFT_eSPI Display::tft;

// LVGL display buffer
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[DISPLAY_WIDTH * 40]; // 320 * 40 buffer

// LVGL input device
static lv_indev_drv_t indev_drv;
static lv_indev_t *indev;

// Touch input
static uint16_t lastX = 0, lastY = 0;
static bool lastTouched = false;
static uint32_t diagnostic_count = 0;

static bool readTouch()
{
    // Check if screen is touched
    if (!ts.touched())
    {
        lastTouched = false;
        return false;
    }
    
    // Read coordinates from XPT2046
    TS_Point p = ts.getPoint();
    
    if (p.z > 0)  // Valid pressure detected
    {
        lastX = p.x;
        lastY = p.y;
        lastTouched = true;
        
        if (++diagnostic_count % 10 == 0) {
            Serial.printf("[TOUCH_RAW] Count=%d X=%d Y=%d Z=%d\n", 
                diagnostic_count, lastX, lastY, p.z);
        }
        
        return true;
    }
    
    lastTouched = false;
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

    // Initialize touchscreen with proper SPI configuration
    // XPT2046_Touchscreen will use pins defined in User_Setup.h
    Serial.printf("%s Initializing XPT2046 touchscreen...\n", TAG);
    ts.begin();
    ts.setRotation(1);  // Match display rotation (1=landscape)
    
    Serial.printf("%s XPT2046 initialized on pins: CS=%d, IRQ=%d\n", 
        TAG, TOUCH_CS, TOUCH_IRQ);
    Serial.printf("%s Touch calibration: X %d-%d, Y %d-%d\n", 
        TAG, TOUCH_X_MIN, TOUCH_X_MAX, TOUCH_Y_MIN, TOUCH_Y_MAX);

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

    // Setup LVGL input device for touchscreen
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = lvgl_touch_cb;
    indev = lv_indev_drv_register(&indev_drv);
    Serial.printf("%s Touch input device registered\n", TAG);

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
    static uint32_t callback_count = 0;
    callback_count++;
    
    bool touched = readTouch();
    
    if (touched) {
        // TFT_eSPI getTouch() returns raw ADC values (0-4095 range)
        // Map to screen coordinates (0-319 x 0-239) using calibration constants
        uint16_t mapped_x = map(lastX, TOUCH_X_MIN, TOUCH_X_MAX, 0, DISPLAY_WIDTH - 1);
        uint16_t mapped_y = map(lastY, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, DISPLAY_HEIGHT - 1);
        
        // Clamp to screen bounds
        mapped_x = constrain(mapped_x, 0, DISPLAY_WIDTH - 1);
        mapped_y = constrain(mapped_y, 0, DISPLAY_HEIGHT - 1);
        
        // Apply rotation transformation for LV_DISP_ROT_270 (270-degree landscape rotation)
        // For 270-degree rotation: x_rotated = y, y_rotated = (width - 1 - x)
        data->point.x = mapped_y;
        data->point.y = (DISPLAY_WIDTH - 1) - mapped_x;
        data->state = LV_INDEV_STATE_PRESSED;
        
        if (callback_count % 100 == 0) {
            Serial.printf("[TOUCH_CB] Raw: X=%d Y=%d | Mapped: X=%d Y=%d | Final: X=%d Y=%d\n", 
                lastX, lastY, mapped_x, mapped_y, data->point.x, data->point.y);
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

