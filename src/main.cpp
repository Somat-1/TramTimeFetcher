#include <Arduino.h>
#include "wifi_mgr.h"
#include "api.h"
#include "disp.h"
#include "config.h"
#include "espnow_receiver.h"
#include "weather.h"
#include <time.h>

#define LED_PIN 8  // Onboard LED on ESP32-C3

unsigned long lastUpdate = 0;
unsigned long lastWeatherUpdate = 0;
unsigned long lastBrightnessUpdate = 0;
Weather currentWeather;

void syncTime() {
    Serial.println("Syncing time with NTP...");
    // CET = UTC+1 (3600 seconds), DST offset = 3600 for summer (but not active in December)
    // For Netherlands: use 3600 offset, 0 DST in winter, 3600 DST in summer
    configTime(3600, 0, "pool.ntp.org", "time.nist.gov");  // CET timezone (UTC+1, no DST in December)
    
    Serial.print("Waiting for NTP time sync");
    int attempts = 0;
    while (time(nullptr) < 100000 && attempts < 30) {
        Serial.print(".");
        delay(500);
        attempts++;
    }
    Serial.println();
    
    time_t now = time(nullptr);
    if (now > 100000) {
        struct tm* ti = localtime(&now);
        Serial.printf("Time synced: %04d-%02d-%02d %02d:%02d:%02d (CET)\n", 
                     ti->tm_year + 1900, ti->tm_mon + 1, ti->tm_mday,
                     ti->tm_hour, ti->tm_min, ti->tm_sec);
    } else {
        Serial.println("WARNING: Time sync failed!");
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=== TramReader Starting ===");
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    // Initialize LED pin and blink 3 times fast to show we're alive
    pinMode(LED_PIN, OUTPUT);
    for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
    }
    
    // Ensure LED is OFF after blinking
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("Initializing display...");
    try {
        initDisplay();
        Serial.println("Display initialized successfully!");
    } catch (...) {
        Serial.println("ERROR: Display initialization failed!");
        while(1) {
            digitalWrite(LED_PIN, !digitalRead(LED_PIN));
            delay(100);  // Fast blink on error
        }
    }
    
    Serial.println("Showing startup message...");
    showMessage("Starting...");
    delay(1000);
    
    showMessage("WiFi...");
    if (connectWiFi()) {
        showMessage("Connected!");
        Serial.println("WiFi OK");
        
        // Sync time after WiFi connection
        showMessage("Sync time...");
        syncTime();
    } else {
        showMessage("WiFi Failed");
        Serial.println("WiFi Failed");
    }
    
    delay(2000);
    
    // Initialize ESP-NOW receiver
    showMessage("ESP-NOW...");
    initESPNowReceiver();
    delay(1000);
    
    // Fetch weather immediately on startup
    showMessage("Weather...");
    Serial.println("Fetching initial weather data...");
    if (fetchWeather(currentWeather)) {
        Serial.println("Weather data loaded successfully!");
    } else {
        Serial.println("Failed to fetch weather on startup");
    }
    delay(1000);
    
    // Ensure LED stays off after setup complete
    digitalWrite(LED_PIN, LOW);
    
    Serial.println("Setup complete! LED should be off.");
    Serial.printf("LED_PIN (GPIO%d) state: %d\n", LED_PIN, digitalRead(LED_PIN));
}

void loop() {
    // Keep LED off during normal operation
    // Only blinked 3 times at startup
    
    // Update brightness every 5 minutes (check if we crossed into/out of night mode)
    if (millis() - lastBrightnessUpdate >= 5 * 60 * 1000) {
        lastBrightnessUpdate = millis();
        updateBrightnessForTime();
    }
    
    // Check WiFi connection and reconnect if needed
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️  WiFi disconnected! Reconnecting...");
        showMessage("WiFi lost...");
        if (connectWiFi()) {
            showMessage("Reconnected!");
            delay(1000);
        } else {
            showMessage("WiFi failed!");
            delay(5000);
            return;
        }
    }
    
    // Update weather every 10 minutes
    if (millis() - lastWeatherUpdate >= WEATHER_UPDATE_INTERVAL) {
        lastWeatherUpdate = millis();
        Serial.println("Fetching weather data...");
        fetchWeather(currentWeather);
    }
    
    if (millis() - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = millis();
        
        Serial.println("\n========================================");
        Serial.println("STARTING TRAM DATA FETCH");
        Serial.printf("WiFi Status: %s (RSSI: %d dBm)\n", 
                     WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected",
                     WiFi.RSSI());
        Serial.println("========================================");
        
        auto trams = fetchTrams();
        
        Serial.println("========================================");
        if (!trams.empty()) {
            Serial.printf("SUCCESS: Got %d trams, displaying now\n", trams.size());
            
            // Check what data we have available
            bool hasWeather = currentWeather.valid;
            
            // Check for data from both sensors
            bool hasOlga = hasSensorData(OLGA_MAC);
            bool hasAE = hasSensorData(AE_MAC);
            
            // Get sensor data with age check (7 hours - longer than 6 hour sensor cycle)
            // This ensures data persists until next reading arrives
            const unsigned long MAX_DATA_AGE = 7UL * 60UL * 60UL * 1000UL;  // 7 hours in milliseconds
            sensor_data_t olgaData = {};
            sensor_data_t aeData = {};
            
            if (hasOlga) {
                unsigned long olgaAge = millis() - getLastReceivedTime(OLGA_MAC);
                if (olgaAge < MAX_DATA_AGE) {
                    olgaData = getSensorData(OLGA_MAC);
                    Serial.printf("Olga data: S=%d%%, B=%d%% (age: %lu min)\n", 
                                 olgaData.soilMoisture, olgaData.batteryPercent, olgaAge/60000);
                } else {
                    Serial.printf("Olga data too old (%lu min), not displaying\n", olgaAge/60000);
                    hasOlga = false;
                }
            } else {
                Serial.println("No data from Olga sensor yet");
            }
            
            if (hasAE) {
                unsigned long aeAge = millis() - getLastReceivedTime(AE_MAC);
                if (aeAge < MAX_DATA_AGE) {
                    aeData = getSensorData(AE_MAC);
                    Serial.printf("A&E data: S=%d%%, B=%d%% (age: %lu min)\n", 
                                 aeData.soilMoisture, aeData.batteryPercent, aeAge/60000);
                } else {
                    Serial.printf("A&E data too old (%lu min), not displaying\n", aeAge/60000);
                    hasAE = false;
                }
            } else {
                Serial.println("No data from A&E sensor yet");
            }
            
            // Always show trams with weather and sensor sections (even if sensor data is missing)
            if (hasWeather) {
                Serial.println("Displaying: Trams + Weather + Multi-Sensor");
                showTramsWithWeatherAndSensor(trams, currentWeather, olgaData, aeData, hasOlga, hasAE);
            } else {
                Serial.println("Displaying: Trams + Multi-Sensor (no weather yet)");
                // For now, still show the full display even without weather
                showTramsWithWeatherAndSensor(trams, currentWeather, olgaData, aeData, hasOlga, hasAE);
            }
        } else {
            Serial.println("ERROR: No trams returned from API");
            showDebugInfo("No data", getLastHttpCode(), getLastHtmlSize(), getLastFoundEntries());
        }
        Serial.println("========================================\n");
    }
    
    delay(100);
}
