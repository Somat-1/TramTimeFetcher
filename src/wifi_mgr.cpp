#include "wifi_mgr.h"
#include "config.h"
#include <WiFi.h>
#include <esp_wifi.h>

// Store BSSID for locking
static uint8_t savedBSSID[6] = {0};
static bool hasSavedBSSID = false;

// WiFi event handler for debugging
void WiFiEvent(WiFiEvent_t event) {
    Serial.printf("[WiFi Event] ");
    switch(event) {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.println("Got IP");
            break;
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.println("Disconnected!");
            break;
        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("Connected!");
            break;
        default:
            Serial.printf("Event: %d\n", event);
            break;
    }
}

bool connectWiFi() {
    Serial.println("\n=== WiFi Connection ===");
    Serial.printf("SSID: %s\n", WIFI_SSID);
    
    // Register WiFi event handler
    WiFi.onEvent(WiFiEvent);
    
    // Disconnect any previous connection
    WiFi.disconnect(true);
    delay(100);
    
    // Set WiFi mode
    WiFi.mode(WIFI_STA);
    delay(100);
    
    // Set hostname
    WiFi.setHostname("TramReader");
    Serial.println("Hostname set to: TramReader");
    
    // Set WiFi power to maximum for better connection
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
    Serial.println("WiFi power set to 19.5dBm (maximum)");
    
    // Disable power saving for stable connection
    esp_wifi_set_ps(WIFI_PS_NONE);
    Serial.println("WiFi power saving disabled");
    
    // Set long range mode for better connectivity
    esp_wifi_set_protocol(WIFI_IF_STA, WIFI_PROTOCOL_11B | WIFI_PROTOCOL_11G | WIFI_PROTOCOL_11N);
    Serial.println("WiFi protocol set to 11bgn");
    
    // First connection: scan and connect
    if (!hasSavedBSSID) {
        Serial.println("First connection - scanning for best AP...");
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    } else {
        // Use saved BSSID for faster reconnection
        Serial.print("Connecting to saved BSSID: ");
        for (int i = 0; i < 6; i++) {
            Serial.printf("%02X%s", savedBSSID[i], i < 5 ? ":" : "");
        }
        Serial.println();
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 0, savedBSSID);
    }
    
    Serial.print("Connecting");
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries++ < 40) {
        Serial.print(".");
        delay(500);
        
        // Print status every 5 seconds
        if (tries % 10 == 0) {
            Serial.printf("\nStatus: %d, RSSI: %d dBm ", WiFi.status(), WiFi.RSSI());
        }
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("✅ WiFi Connected!");
        Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
        Serial.printf("Channel: %d\n", WiFi.channel());
        
        // Save BSSID for future connections
        uint8_t* bssid = WiFi.BSSID();
        if (bssid != nullptr) {
            memcpy(savedBSSID, bssid, 6);
            hasSavedBSSID = true;
            Serial.print("BSSID locked: ");
            for (int i = 0; i < 6; i++) {
                Serial.printf("%02X%s", savedBSSID[i], i < 5 ? ":" : "");
            }
            Serial.println();
        }
        
        return true;
    } else {
        Serial.printf("❌ WiFi Failed! Status: %d\n", WiFi.status());
        
        // Print status details
        switch (WiFi.status()) {
            case WL_NO_SSID_AVAIL:
                Serial.println("SSID not found");
                break;
            case WL_CONNECT_FAILED:
                Serial.println("Connection failed");
                break;
            case WL_CONNECTION_LOST:
                Serial.println("Connection lost");
                break;
            case WL_DISCONNECTED:
                Serial.println("Disconnected");
                break;
            default:
                Serial.println("Unknown error");
        }
        
        // Reset BSSID lock if connection fails
        hasSavedBSSID = false;
        return false;
    }
}
