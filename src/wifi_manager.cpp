#include "wifi_manager.h"
#include "config.h"
#include <esp_wifi.h>

WiFiManager::WiFiManager(int powerDbm) : bssidLocked(false), lastReconnectAttempt(0), wifiPowerDbm(powerDbm) {
    memset(bssid, 0, 6);
}

void WiFiManager::scanNetworks(const char* targetSSID) {
    Serial.println("\n=== Scanning for WiFi networks ===");
    int n = WiFi.scanNetworks();
    
    if (n == 0) {
        Serial.println("No networks found");
    } else {
        Serial.printf("Found %d networks:\n", n);
        Serial.println("Nr | SSID                             | RSSI | Ch | Encryption");
        Serial.println("---|----------------------------------|------|----|------------");
        
        int targetIndex = -1;
        int bestRSSI = -1000;
        
        for (int i = 0; i < n; ++i) {
            String ssid = WiFi.SSID(i);
            int32_t rssi = WiFi.RSSI(i);
            uint8_t encType = WiFi.encryptionType(i);
            
            Serial.printf("%2d | %-32s | %4d | %2d | ", 
                         i, ssid.c_str(), rssi, WiFi.channel(i));
            
            switch(encType) {
                case WIFI_AUTH_OPEN: Serial.print("OPEN"); break;
                case WIFI_AUTH_WEP: Serial.print("WEP"); break;
                case WIFI_AUTH_WPA_PSK: Serial.print("WPA"); break;
                case WIFI_AUTH_WPA2_PSK: Serial.print("WPA2"); break;
                case WIFI_AUTH_WPA_WPA2_PSK: Serial.print("WPA/WPA2"); break;
                case WIFI_AUTH_WPA2_ENTERPRISE: Serial.print("WPA2-EAP"); break;
                default: Serial.print("UNKNOWN");
            }
            
            if (ssid == targetSSID) {
                Serial.print(" <- TARGET");
                if (rssi > bestRSSI) {
                    bestRSSI = rssi;
                    targetIndex = i;
                }
            }
            Serial.println();
        }
        
        if (targetIndex >= 0) {
            Serial.printf("\nBest signal for '%s': %d dBm on channel %d\n", 
                         targetSSID, bestRSSI, WiFi.channel(targetIndex));
            // Store the BSSID of the best AP
            uint8_t* scanBSSID = WiFi.BSSID(targetIndex);
            Serial.print("BSSID: ");
            for (int i = 0; i < 6; i++) {
                Serial.printf("%02X", scanBSSID[i]);
                if (i < 5) Serial.print(":");
            }
            Serial.println();
        } else {
            Serial.printf("\nWARNING: Target SSID '%s' not found in scan!\n", targetSSID);
        }
    }
    
    WiFi.scanDelete();
    Serial.println("=================================\n");
}

bool WiFiManager::connect(const char* ssid, const char* password, bool lockBSSID) {
    Serial.println("\n=== WiFi Connection Process ===");
    
    // Disconnect any previous connection
    WiFi.disconnect(true);
    delay(100);
    
    // Set WiFi mode to Station
    WiFi.mode(WIFI_STA);
    Serial.println("WiFi mode set to STA (Station)");
    
    // CRITICAL: Set WiFi power BEFORE connecting
    // Map dBm value from config to ESP32 power level
    wifi_power_t powerLevel;
    int txPower = wifiPowerDbm * 4;  // ESP32 uses quarter-dBm units
    
    if (wifiPowerDbm >= 19) {
        powerLevel = WIFI_POWER_19_5dBm;
        txPower = 78;
    } else if (wifiPowerDbm >= 18) {
        powerLevel = WIFI_POWER_18_5dBm;
        txPower = 74;
    } else {
        powerLevel = WIFI_POWER_17dBm;
        txPower = 68;
    }
    
    esp_wifi_set_max_tx_power(txPower);
    WiFi.setTxPower(powerLevel);
    Serial.printf("WiFi TX power set to %d dBm (value: %d)\n", wifiPowerDbm, txPower);
    
    // Scan for networks first
    scanNetworks(ssid);
    
    // Set aggressive power save settings to maintain connection
    esp_wifi_set_ps(WIFI_PS_NONE);  // Disable power save for stability
    Serial.println("WiFi power save disabled for stability");
    
    // Start connection
    Serial.printf("Attempting to connect to SSID: '%s'\n", ssid);
    Serial.println("Connection timeout: 10 seconds");
    
    WiFi.begin(ssid, password);
    
    // Wait for connection with detailed status
    int attempts = 0;
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && attempts < 40) {  // 40 * 250ms = 10 seconds
        delay(250);
        
        // Print status every 2 seconds
        if (attempts % 8 == 0) {
            wl_status_t status = WiFi.status();
            Serial.print("\nStatus: ");
            switch(status) {
                case WL_IDLE_STATUS: Serial.print("IDLE"); break;
                case WL_NO_SSID_AVAIL: Serial.print("NO_SSID"); break;
                case WL_SCAN_COMPLETED: Serial.print("SCAN_COMPLETE"); break;
                case WL_CONNECTED: Serial.print("CONNECTED"); break;
                case WL_CONNECT_FAILED: Serial.print("FAILED"); break;
                case WL_CONNECTION_LOST: Serial.print("LOST"); break;
                case WL_DISCONNECTED: Serial.print("DISCONNECTED"); break;
                default: Serial.printf("UNKNOWN(%d)", status); break;
            }
            Serial.print(" - ");
        }
        Serial.print(".");
        attempts++;
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✓ WiFi CONNECTED successfully!");
        Serial.printf("Connection established in %.1f seconds\n", (millis() - startTime) / 1000.0);
        printConnectionInfo();
        
        if (lockBSSID) {
            lockCurrentBSSID();
        }
        
        return true;
    } else {
        Serial.println("\n✗ WiFi CONNECTION FAILED!");
        Serial.print("Final status: ");
        wl_status_t status = WiFi.status();
        switch(status) {
            case WL_NO_SSID_AVAIL: Serial.println("Network not found"); break;
            case WL_CONNECT_FAILED: Serial.println("Wrong password or connection rejected"); break;
            case WL_CONNECTION_LOST: Serial.println("Connection lost during setup"); break;
            case WL_DISCONNECTED: Serial.println("Disconnected"); break;
            default: Serial.printf("Unknown error (%d)\n", status); break;
        }
        return false;
    }
}

bool WiFiManager::isConnected() {
    wl_status_t status = WiFi.status();
    if (status != WL_CONNECTED) {
        Serial.printf("[WiFi Check] Status: ");
        switch(status) {
            case WL_NO_SHIELD: Serial.println("NO_SHIELD"); break;
            case WL_IDLE_STATUS: Serial.println("IDLE"); break;
            case WL_NO_SSID_AVAIL: Serial.println("NO_SSID"); break;
            case WL_SCAN_COMPLETED: Serial.println("SCAN_COMPLETE"); break;
            case WL_CONNECT_FAILED: Serial.println("FAILED"); break;
            case WL_CONNECTION_LOST: Serial.println("CONNECTION_LOST"); break;
            case WL_DISCONNECTED: Serial.println("DISCONNECTED"); break;
            default: Serial.printf("UNKNOWN(%d)\n", status); break;
        }
    }
    return status == WL_CONNECTED;
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
        Serial.println("\n=== WiFi Reconnection Attempt ===");
        
        // Re-apply power settings from config
        wifi_power_t powerLevel = (wifiPowerDbm >= 19) ? WIFI_POWER_19_5dBm : 
                                  (wifiPowerDbm >= 18) ? WIFI_POWER_18_5dBm : WIFI_POWER_17dBm;
        int txPower = wifiPowerDbm * 4;
        esp_wifi_set_max_tx_power(txPower);
        WiFi.setTxPower(powerLevel);
        esp_wifi_set_ps(WIFI_PS_NONE);
        
        if (bssidLocked) {
            Serial.print("Reconnecting to locked BSSID: ");
            for (int i = 0; i < 6; i++) {
                Serial.printf("%02X", bssid[i]);
                if (i < 5) Serial.print(":");
            }
            Serial.println();
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 0, bssid, true);
        } else {
            Serial.println("Reconnecting to SSID (no BSSID lock)");
            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        }
        
        // Wait a bit for connection
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\n✓ Reconnected successfully!");
            printConnectionInfo();
        } else {
            Serial.println("\n✗ Reconnection failed");
        }
    }
}

void WiFiManager::lockCurrentBSSID() {
    if (isConnected()) {
        uint8_t* currentBSSID = WiFi.BSSID();
        memcpy(bssid, currentBSSID, 6);
        bssidLocked = true;
        Serial.println("\n=== BSSID LOCKED ===");
        Serial.print("Locked to BSSID: ");
        for (int i = 0; i < 6; i++) {
            Serial.printf("%02X", bssid[i]);
            if (i < 5) Serial.print(":");
        }
        Serial.println();
        Serial.println("Future reconnections will use this specific access point");
        Serial.println("====================\n");
    }
}

void WiFiManager::printConnectionInfo() {
    Serial.println("\n╔════════════════════════════════════╗");
    Serial.println("║    WiFi Connection Information    ║");
    Serial.println("╚════════════════════════════════════╝");
    
    Serial.print("  SSID:       ");
    Serial.println(WiFi.SSID());
    
    Serial.print("  BSSID:      ");
    uint8_t* currentBSSID = WiFi.BSSID();
    for (int i = 0; i < 6; i++) {
        Serial.printf("%02X", currentBSSID[i]);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
    
    Serial.print("  Channel:    ");
    Serial.println(WiFi.channel());
    
    Serial.print("  IP Address: ");
    Serial.println(WiFi.localIP());
    
    Serial.print("  Gateway:    ");
    Serial.println(WiFi.gatewayIP());
    
    Serial.print("  Subnet:     ");
    Serial.println(WiFi.subnetMask());
    
    Serial.print("  DNS:        ");
    Serial.println(WiFi.dnsIP());
    
    int32_t rssi = WiFi.RSSI();
    Serial.print("  RSSI:       ");
    Serial.print(rssi);
    Serial.print(" dBm (");
    if (rssi > -50) Serial.print("Excellent");
    else if (rssi > -60) Serial.print("Good");
    else if (rssi > -70) Serial.print("Fair");
    else Serial.print("Weak");
    Serial.println(")");
    
    Serial.print("  TX Power:   17 dBm (limited)\n");
    Serial.print("  Power Save: Disabled\n");
    
    Serial.println("════════════════════════════════════\n");
}
