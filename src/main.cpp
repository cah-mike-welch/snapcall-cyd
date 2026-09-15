// CYD LVGL + WiFi API + Dynamic UI (ESP32-2432S028R)
#include "Arduino.h"
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <SPI.h>
#include "esp_timer.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h> // Make sure to add bblanchon/ArduinoJson to lib_deps

// API Configuration
#define BEARER_TOKEN "cRJyrW8Ep16fmBxIuQX7R0LUrMgIwQRdI87drWJEftzRajBrvkqujiPpjAQgSzpQ"

const char *api_url = "https://snapcallapp.com/api/device/v1/clubs";

// Display & Touch Pins
#define TFT_BL 21
#define TFT_CS 15
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33

// Touch Calibration values
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

SPIClass touchSpi = SPIClass(HSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
WiFiMulti wifiMulti;

static lv_obj_t *label_status;
static lv_obj_t *label_wifi;

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
        int16_t x = map(p.x, ts_minx, ts_maxx, 0, SCREEN_WIDTH);
        int16_t y = map(p.y, ts_miny, ts_maxy, 0, SCREEN_HEIGHT);
        x = constrain(x, 0, SCREEN_WIDTH - 1);
        y = constrain(y, 0, SCREEN_HEIGHT - 1);

        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }
}

// Universal button click handler
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        const char *txt = lv_label_get_text(label);

        Serial.printf("[LVGL EVENT] Selected Club: %s\n", txt);

        char status_buf[64];
        snprintf(status_buf, sizeof(status_buf), "Selected: %s", txt);
        lv_label_set_text(label_status, status_buf);
        lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_GREEN), 0);
    }
}

// Builds the static text elements (WiFi IP and Status bar)
void create_base_ui()
{
    label_wifi = lv_label_create(lv_scr_act());
    if (WiFi.status() == WL_CONNECTED)
    {
        String wifiInfo = "WiFi: Connected\nIP: " + WiFi.localIP().toString();
        lv_label_set_text(label_wifi, wifiInfo.c_str());
        lv_obj_set_style_text_color(label_wifi, lv_palette_main(LV_PALETTE_GREEN), 0);
    }
    else
    {
        lv_label_set_text(label_wifi, "WiFi Disconnected");
        lv_obj_set_style_text_color(label_wifi, lv_palette_main(LV_PALETTE_RED), 0);
    }

    lv_obj_set_style_text_align(label_wifi, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(label_wifi, LV_ALIGN_TOP_MID, 0, 5);

    label_status = lv_label_create(lv_scr_act());
    lv_label_set_text(label_status, "Fetching clubs...");
    lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_LEFT, 10, -10);
}

// Makes the API call and dynamically generates buttons
// 1. Try removing the trailing slash here:

void fetch_clubs_and_build_buttons()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        lv_label_set_text(label_status, "Error: No WiFi");
        lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_RED), 0);
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(15);

    HTTPClient http;
    http.setTimeout(15000);

    // 2. Add this line to automatically follow 301/302 redirects
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    Serial.println("[API] Fetching clubs from SnapCall API...");

    if (http.begin(client, api_url))
    {
        http.addHeader("Authorization", String("Bearer ") + BEARER_TOKEN);
        http.addHeader("Accept", "application/json");
        http.addHeader("User-Agent", "ESP32-CYD");

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            Serial.println("[API] Response received successfully!");

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["success"] == true)
            {
                JsonArray clubs = doc["data"]["clubs"];
                int y_offset = 50;

                for (JsonObject club : clubs)
                {
                    const char *short_name = club["short_name"];

                    lv_obj_t *btn = lv_btn_create(lv_scr_act());
                    lv_obj_set_size(btn, 200, 60);
                    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset);
                    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), 0);
                    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, NULL);

                    lv_obj_t *btn_label = lv_label_create(btn);
                    lv_label_set_text(btn_label, short_name);
                    lv_obj_center(btn_label);

                    y_offset += 70;
                }

                lv_label_set_text(label_status, "Ready.");
                lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_GREY), 0);
            }
            else
            {
                Serial.print("[API] JSON Parse failed: ");
                Serial.println(error.c_str());
                lv_label_set_text(label_status, "JSON Parse Error");
                lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_RED), 0);
            }
        }
        else
        {
            Serial.printf("[API] GET request failed, error code: %d (%s)\n", httpCode, http.errorToString(httpCode).c_str());
            char err_buf[32];
            snprintf(err_buf, sizeof(err_buf), "API Error: %d", httpCode);
            lv_label_set_text(label_status, err_buf);
            lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_RED), 0);
        }
        http.end();
    }
    else
    {
        Serial.println("[API] Unable to connect to host");
        lv_label_set_text(label_status, "Connection Failed");
        lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_RED), 0);
    }
}

void setup()
{
    Serial.begin(115200);
    delay(500);

    Serial.println("\n--- Initializing ESP32 CYD ---");

    WiFi.mode(WIFI_STA);
    wifiMulti.addAP("370Katiebud", "mbebmike");
    wifiMulti.addAP("IHC of Cincinnati", "1r1shJig");

    Serial.print("Connecting to WiFi");
    while (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }
    Serial.println("\n[WIFI] Connected!");

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);
    pinMode(XPT2046_IRQ, INPUT_PULLUP);

    tft.init();
    tft.setRotation(0);

    touchSpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(touchSpi);
    ts.setRotation(0);

    lv_init();

    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_tick_cb,
        .name = "lvgl_tick"};
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, 1000);

    lv_disp_draw_buf_init(&draw_buf, buf, NULL, SCREEN_WIDTH * 10);
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = SCREEN_WIDTH;
    disp_drv.ver_res = SCREEN_HEIGHT;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);

    // Build the static UI elements first
    create_base_ui();

    // Call the API and populate the dynamic buttons
    fetch_clubs_and_build_buttons();

    Serial.println("[SYSTEM] Setup complete.");
}

void loop()
{
    if (wifiMulti.run() != WL_CONNECTED)
    {
        Serial.println("[WIFI] Connection lost! Attempting to reconnect...");
        delay(1000);
    }

    lv_timer_handler();
    delay(5);
}