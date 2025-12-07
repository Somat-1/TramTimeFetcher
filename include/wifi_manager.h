#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>

class WiFiManager {
private:
    uint8_t bssid[6];
    bool bssidLocked;
    unsigned long lastReconnectAttempt;
    int wifiPowerDbm;
    
    void scanNetworks(const char* targetSSID);

public:
    WiFiManager(int powerDbm = 17);
    bool connect(const char* ssid, const char* password, bool lockBSSID = true);
    bool isConnected();
    void disconnect();
    void reconnect();
    void lockCurrentBSSID();
    void printConnectionInfo();
};

#endif // WIFI_MANAGER_H
