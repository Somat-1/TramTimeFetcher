#ifndef CONFIG_H
#define CONFIG_H

#define WIFI_SSID "Ceviche2"
#define WIFI_PASSWORD "CapitanoAmericano55"
#define STOP_CODE "NL:S:32000903"
#define STOP_NAME "Statenkwartier"
#define UPDATE_INTERVAL 20000

// Weather API (Open-Meteo - no API key needed!)
#define WEATHER_LAT "52.0767"
#define WEATHER_LON "4.2986"
#define WEATHER_UPDATE_INTERVAL 600000  // 10 minutes

// Display pins (corrected to match actual wiring)
#define TFT_CS    10  // CS  -> GPIO10
#define TFT_RST   3   // RES -> GPIO3
#define TFT_DC    4   // DC  -> GPIO4
#define TFT_MOSI  7   // SDA -> GPIO7 (MOSI)
#define TFT_SCLK  6   // SCK -> GPIO6 (Clock)
#define TFT_BL    1   // BL  -> GPIO1 (Backlight PWM)

// Colors
#define COLOR_BG 0x0000
#define COLOR_TEXT 0xFFFF
#define COLOR_LINE 0xFFE0
#define COLOR_TIME 0x07E0
#define COLOR_TEMP 0xFD20  // Orange
#define COLOR_SENSOR 0x07FF  // Cyan
#define COLOR_GRAY 0x7BEF  // Gray for separator lines (RGB565: 127,127,127)

#endif
