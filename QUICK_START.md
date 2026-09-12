# ⚡ Quick Start Guide

## 5-Minute Setup

### Step 1: Install PlatformIO (2 min)

```
1. Open VS Code
2. Press Ctrl+Shift+X (Extensions)
3. Search for "PlatformIO"
4. Install "PlatformIO IDE" by PlatformIO
5. Restart VS Code
```

### Step 2: Configure Credentials (2 min)

```bash
cd c:\Users\mwelch\source\repos\snapcall-cyd

# Copy template to config file
copy src\config.example.h src\config.h

# Edit src/config.h with your values:
# WIFI_SSID = "Your WiFi Name"
# WIFI_PASSWORD = "Your WiFi Password"
# SNAPCALL_API_KEY = "Your API Key"
```

### Step 3: Build (1 min)

```bash
# In VS Code terminal or command prompt
cd c:\Users\mwelch\source\repos\snapcall-cyd
pio run
```

Expected output: `[OK] Project compiled successfully`

### Step 4: Upload to Device (30 sec)

```bash
# Connect CYD via USB cable
pio run --target upload

# Expected: "Hash of data verified"
```

### Step 5: Monitor Output (30 sec)

```bash
pio device monitor

# You should see:
# [BOOT] SnapCall Dealer Button - Starting
# [WIFI] Connected!
# [WIFI] IP: 192.168.x.x
# [API] Clubs loaded: 4
```

## 🎯 What Happens Next

1. Device boots and shows "Connecting to WiFi..."
2. After ~5 seconds, you see a list of clubs
3. Tap any club to see its current blind levels
4. Blind levels refresh every 12 seconds
5. Tap "BACK" to return to club selection

## ❌ If Something Goes Wrong

### "pio: The term 'pio' is not recognized"

```bash
# Option 1: Use Python module
python -m platformio run

# Option 2: Install PlatformIO CLI
# Visit: https://platformio.org/install/cli
```

### "WiFi connection failed"

1. Check WIFI_SSID and WIFI_PASSWORD in config.h
2. Verify the network is 2.4GHz (not 5GHz)
3. Try a network closer to the device
4. Check serial output: `pio device monitor`

### "Unable to load clubs"

1. Verify SNAPCALL_API_KEY in config.h
2. Check WiFi is actually connected
3. Verify API endpoint is correct
4. Look at serial output for HTTP error codes

### "Touch doesn't work"

1. Tap in different areas of the screen
2. Check serial output for touch coordinates: `[DISPLAY] Touch: x=xxx, y=yyy`
3. May need touch calibration (future feature)

## 📚 Documentation Files

- **README.md** - Complete documentation
- **PROJECT_SUMMARY.md** - Project overview
- **FINAL_CHECKLIST.md** - What's included
- **QUICK_START.md** - This file

## 🔧 Common Commands

```bash
# Build firmware
pio run

# Upload to device
pio run --target upload

# Monitor serial output
pio device monitor

# Clean build files
pio run --target clean

# Show project info
pio project config
```

## 📱 Display What to Expect

### Club Selection Screen

```
        SNAPCALL

      SELECT CLUB

  ┌──────────────────┐
  │ Cincinnati Poker │
  │      Club        │
  └──────────────────┘

  ┌──────────────────┐
  │ Saturday Home    │
  │     Game         │
  └──────────────────┘
```

### Tournament Screen

```
Cincinnati Poker Club

        BLINDS

       100 / 200

      BB Ante: 25

     TIME REMAINING

         14:32

       [ BACK ]
```

## 🎓 Understanding the Code

### Main Application Flow

1. **main.cpp** - State machine that drives the app
2. **SnapCallApi.cpp** - Talks to SnapCall API
3. **Display.cpp** - Manages screen and touch
4. **ClubScreen.cpp** - Shows club list UI
5. **TournamentScreen.cpp** - Shows blind info UI

### How to Find Things

- WiFi connection: Search for "WIFI\_" in main.cpp
- API calls: Look in SnapCallApi.cpp
- Screen layout: Check ClubScreen.cpp and TournamentScreen.cpp
- Display setup: See Display.cpp
- Configuration: Edit src/config.h

## 🚨 Emergency Troubleshooting

If the device is unresponsive:

1. **Power cycle**
   - Unplug USB cable
   - Wait 5 seconds
   - Plug back in

2. **Upload again**
   - Run `pio run --target upload`
   - Make sure device shows in COM port

3. **Check COM port**
   - Look at Device Manager
   - Find "USB-to-UART" device
   - Note the COM number (e.g., COM4)
   - If not found, check USB cable and driver

4. **Last resort**
   - Delete `.pio/` folder
   - Run `pio run --target clean`
   - Run `pio run` again

## 🎉 Success Indicators

You'll know everything is working when:

✓ Serial monitor shows "WiFi Connected!"
✓ You see a list of clubs on the screen
✓ Clubs are touchable (respond to taps)
✓ Tapping a club shows blind levels
✓ "BACK" button works
✓ Blind levels update every ~12 seconds

## 📞 Getting Help

1. Check serial output: `pio device monitor`
2. Look at error messages (shown on screen)
3. Review README.md troubleshooting section
4. Check that config.h values are correct
5. Verify WiFi credentials and API key

## 🎯 You're Ready!

Everything is set up and ready to go. Just:

1. Install PlatformIO
2. Add your credentials to config.h
3. Run `pio run`
4. Upload to device
5. Enjoy!

---

**Questions?** See README.md for detailed documentation.
**First time?** Start with Step 1 and follow each step in order.
**Already built?** Skip to Step 4 (Upload) if firmware didn't change.
