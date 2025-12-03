#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "Ceviche2"
#define WIFI_PASSWORD "CapitanoAmericano55"
#define WIFI_POWER_DBM 17  // Limit WiFi power to 17dB
#define LOCK_BSSID true     // Lock BSSID once connected

// OV API Configuration
#define OV_API_BASE_URL "https://v0.ovapi.nl"
#define STOP_AREA_CODE "732"  // Statenlaan
#define TIMING_POINT_NAME "Statenlaan"

// Update intervals (milliseconds)
#define UPDATE_INTERVAL 30000  // Update every 30 seconds
#define WIFI_RECONNECT_INTERVAL 10000  // Try to reconnect every 10 seconds

// Display Configuration
#define TOUCH_PIN 0  // Adjust based on your hardware
#define MAX_DEPARTURES_DISPLAY 8  // Number of departures to show on screen

// Screen brightness
#define SCREEN_BRIGHTNESS 128  // 0-255

#endif // CONFIG_H
