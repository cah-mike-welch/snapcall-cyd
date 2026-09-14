# Hardware & Firmware Notes — ESP32-2432S028R (CYD)

Working notes from getting the touch button demo running on this board.
Read this before adding new features so you don't re-discover the same quirks.

## Board summary

- **MCU**: ESP32 (WiFi + BT dual core)
- **Display**: 2.8" TFT, ILI9341-compatible clone driver, **native resolution 240 (W) x 320 (H), portrait**
- **Touch**: Resistive, XPT2046 controller, wired on its own SPI bus
- **Two independent SPI buses are required**: the display and the touch controller must NOT share one `SPIClass` instance, or communication is unreliable.

## Pin map

| Function      | Pin | Notes                                                     |
| ------------- | --- | --------------------------------------------------------- |
| TFT MISO      | 12  | HSPI                                                      |
| TFT MOSI      | 13  | HSPI                                                      |
| TFT SCLK      | 14  | HSPI                                                      |
| TFT CS        | 15  | HSPI                                                      |
| TFT DC        | 2   |                                                           |
| TFT RST       | -1  | not wired (tied to EN/reset)                              |
| TFT Backlight | 21  | active HIGH                                               |
| Touch IRQ     | 36  |                                                           |
| Touch MOSI    | 32  | VSPI (bit-banged pins, not the ESP32 default VSPI pinout) |
| Touch MISO    | 39  | VSPI                                                      |
| Touch CLK     | 25  | VSPI                                                      |
| Touch CS      | 33  | VSPI                                                      |

## Display driver: use TFT_eSPI, not Adafruit_ILI9341

We initially built this with `Adafruit_ILI9341` + `Adafruit GFX`. It mostly worked, but a
**persistent, unfixable ~25% band of static/noise** appeared on one edge of the screen
(always the "far" quarter of whichever axis was logically longer), no matter what
`setRotation()` value, screen dimensions, or SPI clock speed (tried down to 20MHz) were used.
A bare `tft.fillScreen()` diagnostic (no app logic at all) still showed it, which ruled out
any coordinate/math bug in our code.

**Root cause**: Adafruit_ILI9341's generic init sequence isn't fully compatible with this
specific clone display revision. The CYD board family has multiple hardware revisions
(v1/v2/v3) with different driver quirks — see `rzeldent/esp32-smartdisplay`'s board list,
which has 3 separate `ESP32-2432S028R` variants for this reason.

**Fix**: switched to `bodmer/TFT_eSPI` configured with the `ILI9341_2_DRIVER` variant
(a driver init tailored for these clone/CYD panels). This completely eliminated the noise.

### TFT_eSPI configuration (via `platformio.ini` `build_flags`, no `User_Setup.h` needed)

```ini
lib_deps =
    bodmer/TFT_eSPI@^2.5.43
    https://github.com/PaulStoffregen/XPT2046_Touchscreen.git
build_flags =
    -D USER_SETUP_LOADED=1
    -D ILI9341_2_DRIVER=1      ; <- the important one; generic ILI9341_DRIVER shows noise on this board
    -D USE_HSPI_PORT=1         ; keeps TFT on HSPI so touch keeps its own dedicated VSPI bus
    -D TFT_WIDTH=240
    -D TFT_HEIGHT=320
    -D TFT_MISO=12
    -D TFT_MOSI=13
    -D TFT_SCLK=14
    -D TFT_CS=15
    -D TFT_DC=2
    -D TFT_RST=-1
    -D TFT_BL=21
    -D TFT_BACKLIGHT_ON=HIGH
    -D SPI_FREQUENCY=40000000
    -D SPI_READ_FREQUENCY=20000000
    -D LOAD_GLCD=1
    -D LOAD_FONT2=1
    -D LOAD_FONT4=1
    -D LOAD_FONT6=1
    -D LOAD_FONT7=1
    -D LOAD_FONT8=1
    -D LOAD_GFXFF=1
    -D SMOOTH_FONT=1
```

Key TFT_eSPI API differences from Adafruit_ILI9341 (in case you're porting old code):

- Construct with no args: `TFT_eSPI tft = TFT_eSPI();` (pins come from the build_flags above)
- Call `tft.init()`, not `tft.begin()`
- Color constants are `TFT_BLACK`, `TFT_WHITE`, `TFT_RED`, `TFT_GREEN`, `TFT_BLUE`, `TFT_YELLOW`, `TFT_CYAN` (not `ILI9341_*`)
- No `getTextBounds()`. For centered text use:
  ```cpp
  tft.setTextDatum(MC_DATUM);       // middle-center anchor
  tft.drawString(text, centerX, centerY);
  tft.setTextDatum(TL_DATUM);       // restore default (top-left) for setCursor/print use elsewhere
  ```

## Orientation: this panel is native PORTRAIT (240 x 320) at `rotation(0)`

Don't fight this — every attempt to force landscape via `setRotation(1)`/`(3)` produced
visually wrong/rotated output on this unit. Keep:

- `tft.setRotation(0)`
- `SCREEN_WIDTH = 240`, `SCREEN_HEIGHT = 320`
- Any UI layout (buttons, calibration targets, message area) must be computed from
  `SCREEN_WIDTH`/`SCREEN_HEIGHT` (or shared constants), never hardcoded pixel literals —
  a stale hardcoded value caused an out-of-bounds calibration target earlier in this project.

If landscape is genuinely needed for a future feature, test carefully and expect to
re-verify all coordinate math — do not assume `rotation(1)` "just works" on this hardware.

## Touch (XPT2046) calibration

- Touch shares no pins/SPI bus with the display (separate `SPIClass vspi = SPIClass(VSPI)`).
- The XPT2046 controller's raw ADC axes do not necessarily line up with the display's axes.
  On an earlier calibration run the raw Y axis came back inverted (`ts_miny > ts_maxy`); the
  code still auto-detects and swaps this if it happens again:
  ```cpp
  if (ts_miny > ts_maxy) { swap(ts_miny, ts_maxy); }
  ```
- **Current hardcoded calibration values** (verified good from a serial log on this unit,
  not inverted this time):
  ```cpp
  uint16_t ts_minx = 489;
  uint16_t ts_miny = 283;
  uint16_t ts_maxx = 3772;
  uint16_t ts_maxy = 3713;
  ```
- The interactive 4-point calibration routine (touch top-left corner, bottom-right corner,
  button 1 center, button 2 center) is still in the code but gated behind a compile-time
  flag so it no longer runs on every boot:
  ```cpp
  #define RUN_TOUCH_CALIBRATION 0   // set to 1 to re-run interactive calibration and print new values
  ```
  If you swap to a different physical unit, or touch accuracy drifts, set this to `1`,
  reflash, read the new `TS_MINX/MAXX/MINY/MAXY` values from the serial monitor (38400 baud),
  paste them back into the `ts_min*/ts_max*` constants, and set the flag back to `0`.
- Touch → screen mapping is a simple linear `map()` per axis; it tolerates either axis being
  "reversed" (min > max) automatically since `map()` handles inverted ranges.

## Known-good current state

- `src/main.cpp`: two-button touch demo, portrait layout, TFT_eSPI, working calibration and
  hit-detection (confirmed via serial `[CHECK] ... HIT/MISS` logs and physical testing).
  Calibration is now hardcoded (see above) instead of running interactively every boot.
- `platformio.ini`: esp32dev env, 38400 baud monitor, TFT_eSPI + XPT2046_Touchscreen deps.
- There is still a **solid-color (RED/GREEN/BLACK) diagnostic fill test** at the top of
  `setup()`, added while debugging the noise issue. It's harmless but adds ~7 seconds to
  every boot. Safe to remove once you're confident the display issue won't resurface.

## Suggested next steps for adding functionality

- Remove/gate the diagnostic fill test and verbose per-touch serial logging behind a
  `#define DEBUG` if you want a quieter production build.
- Consider hardcoding calibration values (from a known-good serial log) and skipping
  `calibrationRoutine()` at boot once your specific unit's values are stable, to speed up
  startup — but keep the routine available (e.g. behind a boot-time touch-and-hold gesture)
  in case the unit's calibration drifts.
- Any new screens/widgets should use `SCREEN_WIDTH`/`SCREEN_HEIGHT` (240/320) for layout math,
  not hardcoded pixel positions.
