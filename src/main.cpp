// filepath: /Users/tomasvalentinas/Documents/PlatformIO/Projects/TramReader/src/main.cpp
#include <Arduino.h>
#include <time.h>
#include "config.h"
#include "wifi_manager.h"
#include "ov_api.h"
#include "display_manager.h"

// Global objects
WiFiManager wifiManager(WIFI_POWER_DBM);
OVApi ovApi(DRGL_BASE_URL, STOP_CODE);
DisplayManager display;

// State variables
unsigned long lastUpdate = 0;
unsigned long lastTouchCheck = 0;
unsigned long lastTouchTime = 0;
bool touchPressed = false;
bool displayIsOn = false;

void setupTime() {
    // Configure time (Amsterdam timezone: CET/CEST)
    configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
    Serial.println("Waiting for NTP time sync...");
    
    time_t now = time(nullptr);
    int attempts = 0;
    while (now < 8 * 3600 * 2 && attempts < 20) {
        delay(500);
        Serial.print(".");
        now = time(nullptr);
        attempts++;
    }
    Serial.println();
    
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
        Serial.print("Current time: ");
        Serial.println(asctime(&timeinfo));
    }
}

void updateDepartures() {
    Serial.println("\n=== Updating departures ===");
    
    if (ovApi.fetchDepartures()) {
        ovApi.printDepartures();
        
        // Only update display if it's on
        if (displayIsOn) {
            display.showUpdating();
            std::vector<Departure> departures = ovApi.getDepartures(MAX_DEPARTURES_DISPLAY);
            display.showDepartures(departures);
        }
        lastUpdate = millis();
    } else {
        Serial.println("Failed to fetch departures");
        if (displayIsOn) {
            display.showError("Failed to fetch data");
            delay(2000);
        }
    }
}

void handleTouch() {
    if (display.isTouched()) {
        Serial.println("Touch detected - turning on display for 5 minutes");
        lastTouchTime = millis();
        
        if (!displayIsOn) {
            display.turnOn();
            displayIsOn = true;
        }
        
        // Force immediate update
        updateDepartures();
    }
}

void checkDisplayTimeout() {
    if (displayIsOn && (millis() - lastTouchTime > DISPLAY_TIMEOUT)) {
        Serial.println("Display timeout - turning off");
        display.turnOff();
        displayIsOn = false;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== Tram Times Display ===");
    Serial.println("ESP32-C3 Supermini");
    Serial.println("Statenkwartier, Den Haag\n");
    
    // Initialize display (touch sensor is initialized inside)
    Serial.println("Initializing display...");
    display.begin();
    displayIsOn = true;  // Start with display on
    lastTouchTime = millis();  // Set initial touch time
    
    display.showWelcomeScreen();
    delay(2000);
    
    // Connect to WiFi
    display.showConnecting();
    if (wifiManager.connect(WIFI_SSID, WIFI_PASSWORD, LOCK_BSSID)) {
        display.showConnected();
        
        // Setup time
        setupTime();
        
        // Initial fetch
        updateDepartures();
    } else {
        display.showError("WiFi connection failed");
        Serial.println("Failed to connect to WiFi");
    }
    
    Serial.println("\n=== Display Configuration ===");
    Serial.println("Display will turn off after 5 minutes of no touch");
    Serial.println("Touch sensor on GPIO 20 to wake display");
    Serial.println("Data updates every 5 seconds");
    Serial.println("Times shown are -1 minute from scheduled");
    Serial.println("==============================\n");
}

void loop() {
    static unsigned long lastConnectionCheck = 0;
    static int disconnectCount = 0;
    
    // Check WiFi connection every 5 seconds (not every loop)
    unsigned long now = millis();
    if (now - lastConnectionCheck > 5000) {
        lastConnectionCheck = now;
        
        if (!wifiManager.isConnected()) {
            disconnectCount++;
            Serial.printf("\n!!! WiFi DISCONNECTED (count: %d) !!!\n", disconnectCount);
            Serial.printf("Uptime: %.1f minutes\n", now / 60000.0);
            Serial.printf("Last successful update: %.1f seconds ago\n", (now - lastUpdate) / 1000.0);
            
            if (displayIsOn) {
                display.showError("WiFi disconnected");
            }
            wifiManager.reconnect();
            delay(2000);
            return;
        }
    }
    
    // Auto-update every UPDATE_INTERVAL (5 seconds - fetch as often as possible)
    if (millis() - lastUpdate > UPDATE_INTERVAL) {
        updateDepartures();
    }
    
    // Check for manual update via touch
    handleTouch();
    
    // Check if display should timeout
    checkDisplayTimeout();
    
    delay(100);
}
