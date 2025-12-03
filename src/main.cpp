#include <Arduino.h>
#include <time.h>
#include "config.h"
#include "wifi_manager.h"
#include "ov_api.h"
#include "display_manager.h"

// Global objects
WiFiManager wifiManager(WIFI_POWER_DBM);
OVApi ovApi(OV_API_BASE_URL, STOP_AREA_CODE);
DisplayManager display;

// State variables
unsigned long lastUpdate = 0;
unsigned long lastTouchCheck = 0;
bool touchPressed = false;

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
    display.showUpdating();
    
    if (ovApi.fetchDepartures()) {
        ovApi.printDepartures();
        std::vector<Departure> departures = ovApi.getDepartures(MAX_DEPARTURES_DISPLAY);
        display.showDepartures(departures);
        lastUpdate = millis();
    } else {
        Serial.println("Failed to fetch departures");
        display.showError("Failed to fetch data");
        delay(2000);
    }
}

void handleTouch() {
    // Check touch sensor (button on GPIO 0 for C3)
    unsigned long now = millis();
    if (now - lastTouchCheck < 200) {
        return; // Debounce
    }
    lastTouchCheck = now;
    
    int touchState = digitalRead(TOUCH_PIN);
    if (touchState == LOW && !touchPressed) {
        touchPressed = true;
        Serial.println("Touch detected - forcing update");
        updateDepartures();
    } else if (touchState == HIGH) {
        touchPressed = false;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\n\n=== Tram Times Display ===");
    Serial.println("ESP32-C3 Supermini");
    Serial.println("Statenlaan, Den Haag\n");
    
    // Initialize touch button
    pinMode(TOUCH_PIN, INPUT_PULLUP);
    
    // Initialize display
    Serial.println("Initializing display...");
    display.begin();
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
}

void loop() {
    // Check WiFi connection
    if (!wifiManager.isConnected()) {
        Serial.println("WiFi disconnected!");
        display.showError("WiFi disconnected");
        wifiManager.reconnect();
        delay(1000);
        return;
    }
    
    // Auto-update every UPDATE_INTERVAL
    if (millis() - lastUpdate > UPDATE_INTERVAL) {
        updateDepartures();
    }
    
    // Check for manual update via touch
    handleTouch();
    
    delay(100);
}