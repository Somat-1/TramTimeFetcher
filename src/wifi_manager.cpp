#include "wifi_manager.h"
#include "config.h"

WiFiManager::WiFiManager(int powerDbm) : bssidLocked(false), lastReconnectAttempt(0), wifiPowerDbm(powerDbm) {
    memset(bssid, 0, 6);
}

bool WiFiManager::connect(const char* ssid, const char* password, bool lockBSSID) {
    Serial.println("Connecting to WiFi...");
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    
    // Set WiFi power
    WiFi.setTxPower(static_cast<wifi_power_t>(wifiPowerDbm * 4)); // Convert dBm to WiFi power enum
    Serial.printf("WiFi power set to %d dBm\n", wifiPowerDbm);
    
    // Start connection
    WiFi.begin(ssid, password);
    
    // Wait for connection
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi connected!");
        printConnectionInfo();
        
        if (lockBSSID) {
            lockCurrentBSSID();
        }
        
        return true;
    } else {
        Serial.println("\nWiFi connection failed!");
        return false;
    }
}

bool WiFiManager::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

void WiFiManager::disconnect() {
    WiFi.disconnect();
    bssidLocked = false;
}

void WiFiManager::reconnect() {
    if (millis() - lastReconnectAttempt < WIFI_RECONNECT_INTERVAL) {
        return;
    }
    
    lastReconnectAttempt = millis();
    
    if (!isConnected()) {
        Serial.println("Attempting to reconnect to WiFi...");
        if (bssidLocked) {
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 0, bssid);
        } else {
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
    }
}

void WiFiManager::lockCurrentBSSID() {
    if (isConnected()) {
        uint8_t* currentBSSID = WiFi.BSSID();
        memcpy(bssid, currentBSSID, 6);
        bssidLocked = true;
        Serial.print("BSSID locked to: ");
        for (int i = 0; i < 6; i++) {
            Serial.printf("%02X", bssid[i]);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
    }
}

void WiFiManager::printConnectionInfo() {
    Serial.println("WiFi Connection Info:");
    Serial.print("  SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("  RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    Serial.print("  BSSID: ");
    uint8_t* currentBSSID = WiFi.BSSID();
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", currentBSSID[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}
