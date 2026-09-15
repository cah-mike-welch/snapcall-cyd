// CYD LVGL + Touch Solution (ESP32-2432S028R)
#include "Arduino.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "esp_timer.h"

// Display Pins
#define TFT_BL 21
#define TFT_CS 15

// Touch Pins
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Dynamic values from your hardware test log
const uint16_t ts_minx = 300;
const uint16_t ts_maxx = 3800;
const uint16_t ts_miny = 400;
const uint16_t ts_maxy = 3800;

#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[SCREEN_WIDTH * 10];
static lv_disp_drv_t disp_drv;
static lv_indev_drv_t indev_drv;

// Separate HSPI instance specifically for touch pins
SPIClass touchSpi = SPIClass(HSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();

static lv_obj_t *label_status;

static void lvgl_tick_cb(void *arg)
{
    lv_tick_inc(1);
}

void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(disp_drv);
}

void my_touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (ts.touched())
    {
        TS_Point p = ts.getPoint();

        // Map raw ADC reads to LVGL display resolution
        int16_t x = map(p.x, ts_minx, ts_maxx, 0, SCREEN_WIDTH);
        int16_t y = map(p.y, ts_miny, ts_maxy, 0, SCREEN_HEIGHT);

        x = constrain(x, 0, SCREEN_WIDTH - 1);
        y = constrain(y, 0, SCREEN_HEIGHT - 1);

        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;

        Serial.printf("[LVGL TOUCH HIT] Raw X:%d Y:%d Z:%d -> Screen X:%d Y:%d\n", p.x, p.y, p.z, x, y);
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        const char *txt = lv_label_get_text(label);

        Serial.printf("[LVGL EVENT] Clicked: %s\n", txt);

        char status_buf[32];
        snprintf(status_buf, sizeof(status_buf), "%s Clicked!", txt);
        lv_label_set_text(label_status, status_buf);
        lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_GREEN), 0);
    }
}

void create_ui()
{
    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "CYD Touch Working!");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);

    // Button A
    lv_obj_t *btn1 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn1, 200, 70);
    lv_obj_align(btn1, LV_ALIGN_TOP_MID, 0, 40);
    lv_obj_set_style_bg_color(btn1, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_add_event_cb(btn1, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn1_label = lv_label_create(btn1);
    lv_label_set_text(btn1_label, "Button A");
    lv_obj_center(btn1_label);

    // Button B
    lv_obj_t *btn2 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(btn2, 200, 70);
    lv_obj_align(btn2, LV_ALIGN_TOP_MID, 0, 130);
    lv_obj_set_style_bg_color(btn2, lv_palette_main(LV_PALETTE_RED), 0);
    lv_obj_add_event_cb(btn2, btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *btn2_label = lv_label_create(btn2);
    lv_label_set_text(btn2_label, "Button B");
    lv_obj_center(btn2_label);

    // Status Label
    label_status = lv_label_create(lv_scr_act());
    lv_label_set_text(label_status, "Awaiting touch input...");
    lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_GREY), 0);
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_LEFT, 10, -10);
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("\n--- Initializing CYD LVGL + Touch ---");

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    pinMode(XPT2046_IRQ, INPUT_PULLUP);

    // 1. Initialize Display
    tft.init();
    tft.setRotation(0);

    // 2. Initialize Dedicated Touch SPI Bus
    touchSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(touchSpi);
    ts.setRotation(0);

    // 3. Initialize LVGL Engine
    lv_init();

    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_cb,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, 1000);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * 10);

    // Display Driver Registration
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Input Driver Registration
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    create_ui();

    Serial.println("[SYSTEM] Setup complete.");
}

void loop()
{
    lv_timer_handler();
    delay(5);
}