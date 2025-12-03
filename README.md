# Tram Times Display - ESP32-C3 Supermini

A real-time tram departure display for Statenkwartier (Tram 17) in Den Haag, Netherlands using DRGL (https://drgl.nl).

## Hardware Requirements

- ESP32-C3 Supermini
- SSD1306 OLED Display (128x64px, I2C)
  - SDA: GPIO 9
  - SCL: GPIO 8
- Touch sensor (GPIO 20)

## Features

- **Real-time tram departures** from DRGL (https://drgl.nl)
- **WiFi connectivity** with power limiting (17dBm) and BSSID locking
- **Compact OLED display** showing:
  - Line number
  - Destination (truncated to fit)
  - Departure time (minutes or HH:MM)
  - Real-time indicator (*)
- **Auto-refresh** every 30 seconds
- **Manual refresh** via touch sensor
- **Time synchronization** via NTP

## Configuration

All configuration is in `include/config.h`:

### WiFi Settings
```cpp
#define WIFI_SSID "Ceviche2"
#define WIFI_PASSWORD "CapitanoAmericano55"
#define WIFI_POWER_DBM 17  // Limited to 17dB
#define LOCK_BSSID true    // Locks to first connected BSSID
```

### Stop Information
```cpp
#define STOP_CODE "NL:S:32000903"  // Statenkwartier (spoor 1)
#define STOP_NAME "Statenkwartier"
```

### Display Pin Configuration
- SDA: GPIO 9
- SCL: GPIO 8
- Touch: GPIO 20
- Screen Address: 0x3C

## Project Structure

```
TramReader/
├── platformio.ini          # PlatformIO configuration
├── include/
│   ├── config.h           # Configuration settings
│   ├── wifi_manager.h     # WiFi management
│   ├── ov_api.h          # OV API interface
│   └── display_manager.h  # Display control
├── src/
│   ├── main.cpp          # Main application
│   ├── wifi_manager.cpp  # WiFi implementation
│   ├── ov_api.cpp       # API implementation
│   └── display_manager.cpp # Display implementation
└── README.md
```

## Building and Uploading

1. Open project in PlatformIO (VS Code)
2. Connect ESP32-C3 Supermini via USB
3. Build: `pio run`
4. Upload: `pio run --target upload`
5. Monitor: `pio device monitor`

## API Information

The project uses DRGL (Dutch Real-time Public Transport):
- Base URL: https://drgl.nl/stop/
- Stop Code: NL:S:32000903 (Statenkwartier, Den Haag)
- Shows Tram 17 to Wateringen

**Why DRGL instead of OVapi?**
- Simpler HTML parsing (no complex JSON)
- More reliable real-time data
- Direct stop lookup without complicated timing point codes

## Display Information

The 128x64 OLED display shows:
- **Header**: Station name (top line with underline)
- **Line number**: 3 chars max, left column
- **Destination**: Truncated to 10 chars, middle
- **Time**: Right column
  - Shows minutes for departures < 60 min (e.g., "5m")
  - Shows HH:MM for later departures
  - "NOW" for immediate departures
- **Asterisk (*)**: Indicates scheduled (non-real-time) data
- **Footer**: "Touch:more" if more departures available

## Troubleshooting

### WiFi Connection Issues

**Error: AUTH_EXPIRE (Reason 2) or TIMEOUT (Reason 39)**
- The WiFi power is limited to 17dBm which may be too low
- Try temporarily increasing to 19.5dBm in `config.h`:
  ```cpp
  #define WIFI_POWER_DBM 19  // Increase from 17
  ```
- Once connected reliably, reduce back to 17dBm
- Ensure you're close to the WiFi router during initial setup
- Verify the password is correct (case-sensitive)
- Check that 2.4GHz WiFi is available (ESP32 doesn't support 5GHz)

### Display Issues
- **No display**: Check I2C connections (SDA=9, SCL=8)
- **Wrong address**: Try 0x3D if 0x3C doesn't work
- **Garbled text**: Verify screen dimensions (128x64)

### No Tram Data
- Check WiFi connection status in serial monitor
- Verify HTTPS works (WiFiClientSecure with setInsecure())
- Test DRGL URL manually: https://drgl.nl/stop/NL:S:32000903
- Check if Tram 17 is running (service hours)
- Monitor serial output for connection details

### Display Issues
- Verify pin connections match `platformio.ini`
- Check display model (configured for ST7789)
- Adjust rotation in `display_manager.cpp` if needed

### API Issues
- Check internet connectivity
- Verify OV API is accessible
- Monitor serial output for API responses
- Check if stop area code is correct

## Touch Button

Press the touch button (GPIO 0) to:
- Manually refresh departure times
- Wake display (if sleep mode added in future)

## Future Enhancements

- [ ] Sleep mode to save power
- [ ] Battery level indicator
- [ ] Multiple stop support
- [ ] Departure alerts/notifications
- [ ] Configurable update intervals
- [ ] Weather information
- [ ] WiFi configuration via web portal

## License

MIT License

## Credits

- OV API: https://github.com/koch-t/KV78Turbo-OVAPI
- TFT_eSPI library by Bodmer
- ArduinoJson by Benoit Blanchon
