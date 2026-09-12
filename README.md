# SnapCall Dealer Button MVP

## Overview

The SnapCall Dealer Button is a compact ESP32-based touchscreen device that displays poker tournament blind levels and allows selection of different poker clubs. It connects to the SnapCall poker tournament management API to fetch and display current tournament information.

## Hardware

### Target Device

**Cheap Yellow Display (CYD)** based on ESP32-D0WDQ6

### Display

- **Model**: ILI9341
- **Resolution**: 320 x 240 pixels (landscape)
- **Size**: 2.8 inch
- **Type**: Color TFT with touchscreen

### Touch Controller

- **Model**: XPT2046
- **Interface**: SPI

### Pin Configuration

```
TFT Display:
  MOSI = GPIO 13
  MISO = GPIO 12
  SCLK = GPIO 14
  CS   = GPIO 15
  DC   = GPIO 2
  RST  = -1 (not used)
  BL   = GPIO 21 (backlight)

Touchscreen:
  CS = GPIO 33
```

## Development Environment

- **IDE**: Visual Studio Code
- **Build System**: PlatformIO
- **Framework**: Arduino for ESP32
- **Language**: C++

### Required Software

1. Visual Studio Code
2. PlatformIO extension for VS Code
3. ESP32 board support

### Installation

1. **Install Visual Studio Code** from https://code.visualstudio.com/

2. **Install PlatformIO**
   - Open VS Code
   - Go to Extensions (Ctrl+Shift+X)
   - Search for "PlatformIO"
   - Click "Install" on the official PlatformIO IDE extension

3. **Clone the Repository**
   ```bash
   git clone <repository-url>
   cd snapcall-dealer-button
   ```

## Configuration

### Step 1: Create `src/config.h`

The project requires a local configuration file that is NOT committed to Git. A template is provided in `src/config.example.h`.

1. Copy `src/config.example.h` to `src/config.h`
2. Edit `src/config.h` and fill in your values:

```cpp
#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

#define SNAPCALL_API_BASE_URL "http://snapcallapp.com/api/device/v1"
#define SNAPCALL_API_KEY "YOUR_API_KEY"
```

**Important**: Do NOT commit `src/config.h` to Git. This file is already in `.gitignore`.

## Building

### Build the Project

```bash
pio run
```

This will:

- Download required libraries (first build only)
- Compile the firmware
- Create a binary file ready for upload

### Expected Build Output

```
Building...
[OK] Project compiled successfully
```

## Uploading

### Connect the Device

1. Plug the CYD into your computer via USB cable
2. The device should appear as a COM port (Windows: COM3, COM4, etc.; Linux/Mac: /dev/ttyUSB0)

### Upload Firmware

```bash
pio run --target upload
```

If you need to specify a different COM port:

```bash
pio run --target upload --upload-port COM4
```

### Verify Upload

If successful, you should see:

```
Wrote 1234567 bytes to flash in 12.34 seconds
Hash of data verified.
```

## Serial Monitor

To view debug output and troubleshoot:

```bash
pio device monitor
```

Or with a specific COM port:

```bash
pio device monitor --port COM3
```

**Default baud rate**: 115200

### Example Serial Output

```
[BOOT] SnapCall Dealer Button - Starting
[DISPLAY] Initializing display
[DISPLAY] TFT initialized
[DISPLAY] Backlight initialized
[DISPLAY] LVGL initialized
[DISPLAY] Touchscreen initialized
[DISPLAY] Display initialization complete
[BOOT] WiFi Connecting...
[WIFI] Connected!
[WIFI] IP: 192.168.1.100
[API] GET /clubs/
[API] HTTP 200
[API] success=true
[API] Clubs loaded: 4
```

## API Endpoints

The device communicates with the SnapCall API at:

```
http://snapcallapp.com/api/device/v1/
```

### Endpoints Implemented

#### 1. Get Clubs

```
GET /clubs/
Authorization: Bearer <API_KEY>
```

**Response**:

```json
{
  "success": true,
  "data": {
    "clubs": [
      {
        "id": 1,
        "short_name": "CPC",
        "name": "Cincinnati Poker Club"
      }
    ]
  }
}
```

#### 2. Get Current Blinds

```
GET /clubs/{clubId}/current-blinds
Authorization: Bearer <API_KEY>
```

**Response**:

```json
{
  "success": true,
  "data": {
    "small_blind": "100",
    "big_blind": "200",
    "big_blind_ante": "0",
    "time_remaining": "30:00"
  }
}
```

## Troubleshooting

### WiFi Connection Issues

**Symptoms**: Device shows "WiFi connection failed"

**Solutions**:

1. Verify WIFI_SSID and WIFI_PASSWORD in config.h are correct
2. Ensure the WiFi network is 2.4GHz (ESP32 doesn't support 5GHz)
3. Check that WiFi is broadcasting the SSID (not hidden)
4. Move the device closer to the router
5. Check serial output for IP address assignment

**Serial Debug Output**:

```
[WIFI] Connecting to WiFi: MY_SSID
[WIFI] Connected!
[WIFI] IP: 192.168.1.100
```

### Touchscreen Not Working

**Symptoms**: Buttons don't respond to taps

**Solutions**:

1. Verify XPT2046 CS pin is GPIO 33 (check pin configuration in Display.cpp)
2. Try calibrating the touch input (may be needed based on screen orientation)
3. Check serial output for touch coordinates:
   ```
   [DISPLAY] Touch: x=150, y=120
   ```
4. Try tapping in different areas to verify responsiveness

### Display Not Showing

**Symptoms**: Black screen or no output

**Solutions**:

1. Check USB cable connection
2. Verify the device powers on (LED should light)
3. Ensure TFT pin configuration matches hardware:
   - MOSI=13, MISO=12, SCLK=14, CS=15, DC=2
4. Try uploading the firmware again
5. Check platformio.ini board selection (should be esp32dev)

### API Connection Failures

**Symptoms**: "Unable to load clubs" error

**Solutions**:

1. Verify WiFi is connected (check serial output)
2. Verify SNAPCALL_API_BASE_URL is correct
3. Verify SNAPCALL_API_KEY is valid
4. Check that the API is accessible from your network
5. Look at serial output for HTTP error codes:
   ```
   [API] HTTP 404  <- Endpoint not found
   [API] HTTP 401  <- Unauthorized (bad API key)
   [API] HTTP 500  <- Server error
   ```

## Project Structure

```
snapcall-dealer-button/
├── platformio.ini           # PlatformIO project configuration
├── .gitignore              # Git ignore file
├── README.md               # This file
├── src/
│   ├── main.cpp            # Application entry point & state machine
│   ├── config.h            # Local configuration (not in git)
│   ├── config.example.h    # Configuration template
│   ├── Models.h            # Data structures (Club, BlindLevel)
│   ├── SnapCallApi.h       # API interface
│   ├── SnapCallApi.cpp     # API implementation
│   ├── Display.h           # Display/LVGL management
│   ├── Display.cpp         # Display implementation
│   ├── ClubScreen.h        # Club selection UI
│   ├── ClubScreen.cpp      # Club selection implementation
│   ├── TournamentScreen.h  # Tournament status UI
│   └── TournamentScreen.cpp# Tournament status implementation
├── include/                # (Optional) Additional headers
└── lib/                    # (Optional) Local libraries
```

## Application Flow

1. **Boot**: Initialize display, touchscreen, LVGL
2. **WiFi Connection**: Connect to configured network
3. **Load Clubs**: Fetch club list from SnapCall API
4. **Club Selection**: Display clubs as scrollable list, wait for user tap
5. **Load Tournament**: Fetch current blind/time for selected club
6. **Tournament View**: Display blinds, ante, and remaining time
7. **Periodic Refresh**: Update tournament data every 12 seconds
8. **Back Button**: Return to club selection

## Libraries Used

- **TFT_eSPI** - ILI9341 display driver
- **LVGL** - Graphical UI framework
- **ArduinoJson** - JSON parsing
- **XPT2046_Touchscreen** - Touch input driver
- **ESP32 WiFi/HTTP** - Network communication

## Current Limitations (MVP)

This is a minimum viable product. The following features are NOT implemented:

- ❌ Player management
- ❌ Table selection
- ❌ Player elimination
- ❌ Wi-Fi configuration UI
- ❌ Device settings/preferences
- ❌ Sleep/wake modes
- ❌ OTA firmware updates
- ❌ Audio feedback
- ❌ Screen rotation/orientation changes
- ❌ Advanced animations
- ❌ Local database/caching
- ❌ Bluetooth connectivity

## Future Features

Planned enhancements:

- [ ] Display multiple tournaments per club
- [ ] Show current big blind
- [ ] Enhanced UI theming
- [ ] Simulator for testing without hardware
- [ ] Additional API endpoints
- [ ] Settings menu
- [ ] Device identification/registration

## Build Details

### PlatformIO Environment

```ini
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
board_build.f_cpu = 240000000L
```

### Compile Flags

Key hardware configuration flags in platformio.ini:

- `TFT_WIDTH=320, TFT_HEIGHT=240` - Display resolution
- `TFT_MOSI=13, TFT_MISO=12, TFT_SCLK=14, TFT_CS=15, TFT_DC=2` - Display pins
- `TFT_BL=21` - Backlight pin
- `TOUCH_CS=33` - Touch controller pin

## Debugging

### Serial Logging

The application uses Serial logging with tag prefixes:

- `[BOOT]` - Startup and initialization
- `[DISPLAY]` - Display and LVGL events
- `[WIFI]` - WiFi connection
- `[API]` - API calls and responses
- `[UI-CLUBS]` - Club selection screen
- `[UI-TOURNAMENT]` - Tournament display screen

### Viewing Logs

```bash
# See all logs
pio device monitor

# Filter for specific component
pio device monitor | grep "\[API\]"
```

## Support & Issues

For issues or questions:

1. Check the Troubleshooting section above
2. Review serial output for error messages
3. Verify all configuration values in config.h
4. Ensure hardware is properly connected

## License

[Project License - To be determined]

## Author

SnapCall Development Team
