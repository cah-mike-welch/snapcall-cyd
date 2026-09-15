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

// --- App State ---
int selected_club_id = 0;
String selected_club_short_name = "";
String selected_club_full_name = "";

// Structure to attach to each button
struct ClubData
{
    int id;
    char short_name[32];
    char full_name[64];
};

// --- Table State ---
int selected_tournament_id = 0;
int selected_table_id = 0;
int selected_table_number = 0;

// Structure to attach to each table button
struct TableData
{
    int table_id;
    int table_number;
    int tournament_id;
};

SPIClass touchSpi = SPIClass(HSPI);
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
TFT_eSPI tft = TFT_eSPI();
WiFiMulti wifiMulti;

static lv_obj_t *label_status;
static lv_obj_t *label_wifi;

// ==========================================
// HARDWARE / DISPLAY CALLBACKS
// ==========================================
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

// ==========================================
// BASE UI
// ==========================================
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
    lv_label_set_text(label_status, "Fetching data...");
    lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_align(label_status, LV_ALIGN_BOTTOM_LEFT, 10, -10);
}

// ==========================================
// TABLE FUNCTIONS
// ==========================================
static void table_btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        TableData *data = (TableData *)lv_event_get_user_data(e);

        if (data)
        {
            selected_tournament_id = data->tournament_id;
            selected_table_id = data->table_id;
            selected_table_number = data->table_number;

            Serial.println("\n--- Table Selected ---");
            Serial.printf("Tournament ID: %d\n", selected_tournament_id);
            Serial.printf("Table ID: %d\n", selected_table_id);
            Serial.printf("Table Number: %d\n", selected_table_number);

            char status_buf[64];
            snprintf(status_buf, sizeof(status_buf), "Table %d Selected", data->table_number);
            lv_label_set_text(label_status, status_buf);
            lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_GREEN), 0);

            // TODO: Trigger your next screen/API call here!
        }
    }
}

// Makes the API call and dynamically generates table buttons
void fetch_tables_and_build_buttons()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        lv_label_set_text(label_status, "Error: No WiFi");
        return;
    }

    // 1. Clear the screen of the previous club buttons
    lv_obj_clean(lv_scr_act());
    // Re-draw the static header/footer
    create_base_ui();

    lv_label_set_text(label_status, "Fetching tables...");

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(15);

    HTTPClient http;
    http.setTimeout(15000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    // 2. Build the dynamic URL using the selected_club_id
    char table_api_url[128];
    snprintf(table_api_url, sizeof(table_api_url), "https://snapcallapp.com/api/device/v1/clubs/%d/current-tournament", selected_club_id);

    Serial.printf("[API] Fetching tables from: %s\n", table_api_url);

    if (http.begin(client, table_api_url))
    {
        http.addHeader("Authorization", String("Bearer ") + BEARER_TOKEN);
        http.addHeader("Accept", "application/json");
        http.addHeader("User-Agent", "ESP32-CYD");

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["success"] == true)
            {

                int tourney_id = doc["data"]["tournament"]["id"] | 0;
                const char *tourney_name = doc["data"]["tournament"]["name"] | "Current Tournament";

                lv_obj_t *title_label = lv_label_create(lv_scr_act());
                char title_buf[64];
                snprintf(title_buf, sizeof(title_buf), "%s - Select Table", tourney_name);
                lv_label_set_text(title_label, title_buf);
                lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 40);

                JsonArray tables = doc["data"]["tournament"]["tables"];
                int y_offset = 70;

                for (JsonObject table : tables)
                {
                    TableData *tableData = new TableData();
                    tableData->tournament_id = tourney_id;
                    tableData->table_id = table["id"] | 0;
                    tableData->table_number = table["table_number"] | 0;

                    lv_obj_t *btn = lv_btn_create(lv_scr_act());
                    lv_obj_set_size(btn, 200, 60);
                    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset);
                    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_ORANGE), 0);

                    lv_obj_add_event_cb(btn, table_btn_event_cb, LV_EVENT_CLICKED, tableData);

                    lv_obj_t *btn_label = lv_label_create(btn);
                    char btn_text[32];
                    snprintf(btn_text, sizeof(btn_text), "Table %d", tableData->table_number);
                    lv_label_set_text(btn_label, btn_text);
                    lv_obj_center(btn_label);

                    y_offset += 70;
                }

                lv_label_set_text(label_status, "Select a table.");
                lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_GREY), 0);
            }
            else
            {
                Serial.println("[API] JSON Parse failed for tables");
                lv_label_set_text(label_status, "Parse Error");
            }
        }
        else
        {
            Serial.printf("[API] GET tables failed, code: %d\n", httpCode);
            lv_label_set_text(label_status, "API Error");
        }
        http.end();
    }
}

// ==========================================
// CLUB FUNCTIONS
// ==========================================
// Universal button click handler for clubs
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        ClubData *data = (ClubData *)lv_event_get_user_data(e);

        if (data)
        {
            selected_club_id = data->id;
            selected_club_short_name = String(data->short_name);
            selected_club_full_name = String(data->full_name);

            Serial.println("\n--- Club Selected ---");
            Serial.printf("ID: %d\n", selected_club_id);
            Serial.printf("Short Name: %s\n", selected_club_short_name.c_str());
            Serial.printf("Full Name: %s\n", selected_club_full_name.c_str());

            char status_buf[64];
            snprintf(status_buf, sizeof(status_buf), "Active: %s", data->short_name);
            lv_label_set_text(label_status, status_buf);
            lv_obj_set_style_text_color(label_status, lv_palette_main(LV_PALETTE_GREEN), 0);

            // Fetch the tables for this club and rebuild the UI
            fetch_tables_and_build_buttons();
        }
    }
}

// Fetch clubs and dynamically generate buttons
void fetch_clubs_and_build_buttons()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        lv_label_set_text(label_status, "Error: No WiFi");
        return;
    }

    WiFiClientSecure client;
    client.setInsecure();
    client.setTimeout(15);

    HTTPClient http;
    http.setTimeout(15000);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    if (http.begin(client, api_url))
    {
        http.addHeader("Authorization", String("Bearer ") + BEARER_TOKEN);
        http.addHeader("Accept", "application/json");
        http.addHeader("User-Agent", "ESP32-CYD");

        int httpCode = http.GET();

        if (httpCode == HTTP_CODE_OK)
        {
            String payload = http.getString();
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["success"] == true)
            {
                lv_obj_t *title_label = lv_label_create(lv_scr_act());
                lv_label_set_text(title_label, "Select a Club");
                lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 40);

                JsonArray clubs = doc["data"]["clubs"];
                int y_offset = 70;

                for (JsonObject club : clubs)
                {
                    ClubData *clubData = new ClubData();
                    clubData->id = club["id"] | 0;
                    strncpy(clubData->short_name, club["short_name"] | "Unknown", sizeof(clubData->short_name) - 1);
                    strncpy(clubData->full_name, club["name"] | "Unknown", sizeof(clubData->full_name) - 1);

                    lv_obj_t *btn = lv_btn_create(lv_scr_act());
                    lv_obj_set_size(btn, 200, 60);
                    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y_offset);
                    lv_obj_set_style_bg_color(btn, lv_palette_main(LV_PALETTE_BLUE), 0);

                    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_CLICKED, clubData);

                    lv_obj_t *btn_label = lv_label_create(btn);
                    lv_label_set_text(btn_label, clubData->short_name);
                    lv_obj_center(btn_label);

                    y_offset += 70;
                }
                lv_label_set_text(label_status, "Ready.");
            }
        }
        http.end();
    }
}

// ==========================================
// MAIN SETUP & LOOP
// ==========================================
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