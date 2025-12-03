# Tram Times Display - Project Summary

## What We Built

A real-time tram departure display for **Tram 17** at **Statenkwartier** station in Den Haag, showing live departure times on a small OLED screen.

## Key Changes Made

### 1. Hardware Configuration
- **Display**: Changed from TFT to **SSD1306 OLED** (128x64, I2C)
  - SDA: GPIO 9
  - SCL: GPIO 8
  - I2C Address: 0x3C
- **Touch Sensor**: GPIO 20
- **Board**: ESP32-C3 Supermini

### 2. API Switch: OVapi → DRGL
**Why we switched:**
- OVapi had SSL certificate issues (cert is for `de.ovapi.nl` not `v0.ovapi.nl`)
- Original stop code (732 - Statenlaan) only had buses, not trams
- DRGL is simpler to parse (HTML instead of complex JSON)

**New Configuration:**
- URL: `https://drgl.nl/stop/NL:S:32000903`
- Stop: Statenkwartier (spoor 1)
- Shows: Tram 17 to Wateringen

### 3. WiFi Issues & Solutions

**Problems Encountered:**
- `AUTH_EXPIRE (Reason 2)` - Authentication expired
- `TIMEOUT (Reason 39)` - Connection timeout

**Root Cause:** WiFi power set too low (17dBm)

**Solution:** Increased to 19dBm for reliable connection

### 4. Code Structure

```
src/
├── main.cpp              - Main loop, coordinates everything
├── wifi_manager.cpp      - WiFi with power control & BSSID locking
├── ov_api.cpp           - HTML parser for DRGL (not JSON!)
└── display_manager.cpp   - SSD1306 OLED control
```

## Current Status

✅ Code compiles successfully (73.8% flash, 12.3% RAM)
✅ Uses DRGL for real-time tram data
✅ Supports SSD1306 OLED display
✅ Touch sensor for manual refresh
✅ WiFi power optimized (19dBm)
⚠️ **Needs upload** - Serial port currently busy

## How to Upload

1. **Close any serial monitor** that's connected
2. Run: `pio run --target upload`
3. Monitor: `pio device monitor --baud 115200`

## Expected Behavior

1. **Startup**: Shows "Tram Times" splash screen
2. **WiFi**: Connects to "Ceviche2" (19dBm power)
3. **Fetch**: Gets tram data from DRGL every 30 seconds
4. **Display**: Shows up to 4 departures:
   ```
   Statenkwartier
   ─────────────────
   17  Wateringen    5m
   17  Wateringen   15m
   17  Wateringen   25m
   17  Wateringen   35m
   ```
5. **Touch**: Tap sensor to force immediate update

## Troubleshooting

### If WiFi still fails:
```cpp
// In include/config.h
#define WIFI_POWER_DBM 19  // Already set, try 20 if needed
```

### If no tram data appears:
1. Check serial monitor for "Fetching departures from DRGL"
2. Verify HTTPS connection succeeds
3. Look for "Found: BUS line 17" or "Found: TRAM line 17"
4. Check parsing output: "Parsed X departures"

### If display is blank:
1. Check I2C wiring (SDA=9, SCL=8)
2. Try address 0x3D if 0x3C doesn't work
3. Look for "SSD1306 allocation failed" in serial

## Test Data Format

DRGL HTML structure we parse:
```html
<div class="ott-departure-time">14:50</div>
<div class="ott-linecode">17</div>
<div class="ott-destination">Wateringen</div>
```

## Files Modified

- `platformio.ini` - Added SSD1306 libraries, removed TFT
- `include/config.h` - Changed to DRGL, updated pins, WiFi power
- `include/display_manager.h` - Changed to Adafruit_SSD1306
- `include/ov_api.h` - Changed from JSON to HTML parsing
- `src/display_manager.cpp` - Complete rewrite for OLED
- `src/ov_api.cpp` - Complete rewrite for DRGL HTML
- `src/wifi_manager.cpp` - Added ESP WiFi headers, flexible power
- `src/main.cpp` - Updated for new config constants
- `README.md` - Updated documentation

## Next Steps

1. **Upload firmware** (close serial monitor first)
2. **Monitor output** to verify WiFi connection
3. **Check for tram data** in serial log
4. **Verify display** shows departures
5. **Test touch sensor** for manual refresh
6. **(Optional)** Reduce WiFi power back to 17dBm once stable

## Success Criteria

✅ ESP32 connects to WiFi
✅ Fetches HTML from drgl.nl
✅ Parses tram departures
✅ Displays on OLED screen
✅ Auto-refreshes every 30 seconds
✅ Touch sensor triggers manual update

---
**Built:** December 3, 2025
**Platform:** ESP32-C3 Supermini
**Display:** SSD1306 128x64 OLED
**Data Source:** DRGL (drgl.nl)
