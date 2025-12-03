# Tram Times Display - ESP32-C3 Supermini

A real-time tram departure display for Statenlaan station in Den Haag, Netherlands using the OV API.

## Hardware Requirements

- ESP32-C3 Supermini
- TFT Display (ST7789, 135x240px)
- Touch sensor/button (GPIO 0)

## Features

- **Real-time tram departures** from OV API (https://v0.ovapi.nl)
- **WiFi connectivity** with power limiting (17dBm) and BSSID locking
- **Color-coded display**:
  - Red: Departing soon (< 5 min)
  - Green: Departing soon (5-10 min)
  - Cyan: Later departures
  - Yellow: Line numbers
  - Grey asterisk (*): Scheduled times (not real-time)
- **Auto-refresh** every 30 seconds
- **Manual refresh** via touch button
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
#define STOP_AREA_CODE "732"  // Statenlaan
#define TIMING_POINT_NAME "Statenlaan"
```

### Display Pin Configuration
Current pin mapping (adjust in `platformio.ini` if needed):
- MOSI: GPIO 7
- SCLK: GPIO 6
- CS: GPIO 10
- DC: GPIO 2
- RST: GPIO 3
- Touch: GPIO 0 (built-in button)

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

The project uses the OV API (Open Public Transport API):
- Base URL: https://v0.ovapi.nl
- Endpoint: `/stopareacode/{code}`
- Stop Area Code: 732 (Statenlaan, Den Haag)

## Display Information

The display shows:
- **Header**: Station name (orange background)
- **Line number**: Yellow, left column
- **Destination**: White, middle column
- **Time**: Right column
  - Shows minutes for departures < 60 min
  - Shows HH:MM for later departures
  - "NOW" for immediate departures
- **Footer**: Legend for scheduled times

## Troubleshooting

### WiFi Connection Issues
- Check SSID and password in `config.h`
- Verify 2.4GHz WiFi is available (ESP32 doesn't support 5GHz)
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
