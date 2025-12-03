// High Power WiFi Configuration for Troubleshooting
// Use this if you're experiencing AUTH_EXPIRE or TIMEOUT errors
// Copy this to config.h and rebuild

#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration - HIGH POWER VERSION
#define WIFI_SSID "Ceviche2"
#define WIFI_PASSWORD "CapitanoAmericano55"
#define WIFI_POWER_DBM 19  // Maximum power for better connection
#define LOCK_BSSID true     // Lock BSSID once connected

// DRGL API Configuration (simpler than OVapi)
#define DRGL_BASE_URL "https://drgl.nl/stop/"
#define STOP_CODE "NL:S:32000903"  // Statenkwartier (spoor 1)
#define STOP_NAME "Statenkwartier"

// Update intervals (milliseconds)
#define UPDATE_INTERVAL 30000  // Update every 30 seconds
#define WIFI_RECONNECT_INTERVAL 10000  // Try to reconnect every 10 seconds

// Display Configuration
#define SDA_PIN 9
#define SCL_PIN 8
#define TOUCH_PIN 20
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define MAX_DEPARTURES_DISPLAY 4  // Number of departures to show on screen (limited by small screen)

#endif // CONFIG_H
