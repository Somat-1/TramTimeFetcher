#include "espnow_receiver.h"
#include <esp_now.h>
#include <WiFi.h>
#include <map>

// Storage for multiple sensors - key is MAC address as uint64_t
static std::map<uint64_t, sensor_data_t> sensorDataMap;
static std::map<uint64_t, unsigned long> lastReceivedTimeMap;

// Convert MAC array to uint64_t for use as map key
uint64_t macToUint64(const uint8_t* mac) {
  uint64_t result = 0;
  for (int i = 0; i < 6; i++) {
    result = (result << 8) | mac[i];
  }
  return result;
}

// Get sensor name from MAC
const char* getSensorName(uint64_t macAddress) {
  if (macAddress == OLGA_MAC) return "Olga";
  if (macAddress == AE_MAC) return "A&E";
  return "Unknown";
}

// Callback when ESP-NOW data is received
void onDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  if (data_len == sizeof(sensor_data_t)) {
    uint64_t macKey = macToUint64(mac_addr);
    sensor_data_t receivedData;
    memcpy(&receivedData, data, sizeof(sensor_data_t));
    
    // Store data in map
    sensorDataMap[macKey] = receivedData;
    lastReceivedTimeMap[macKey] = millis();
    
    const char* sensorName = getSensorName(macKey);
    
    Serial.println("\n=== ESP-NOW Data Received ===");
    Serial.printf("From: %02X:%02X:%02X:%02X:%02X:%02X (%s)\n",
                  mac_addr[0], mac_addr[1], mac_addr[2],
                  mac_addr[3], mac_addr[4], mac_addr[5], sensorName);
    Serial.printf("Battery: %.2fV (%d%%)\n", 
                  receivedData.batteryVoltage,
                  receivedData.batteryPercent);
    Serial.printf("Soil Moisture: %d%%\n", receivedData.soilMoisture);
    Serial.printf("Timestamp: %lu ms\n", receivedData.timestamp);
    Serial.println("============================\n");
  }
}

void initESPNowReceiver() {
  Serial.println("Initializing ESP-NOW receiver...");
  Serial.println("Waiting for sensors:");
  Serial.println("  - Olga (MAC: 80:F1:B2:50:29:74)");
  Serial.println("  - A&E  (MAC: 80:F1:B2:50:29:48)");
  
  // ESP-NOW must be initialized after WiFi is set up
  if (esp_now_init() != ESP_OK) {
    Serial.println("ERROR: ESP-NOW init failed!");
    return;
  }
  
  Serial.println("ESP-NOW initialized successfully");
  
  // Register receive callback
  esp_now_register_recv_cb(onDataRecv);
  
  Serial.println("ESP-NOW receiver ready, waiting for sensor data...");
}

// Check if specific sensor has data
bool hasSensorData(uint64_t macAddress) {
  return sensorDataMap.find(macAddress) != sensorDataMap.end();
}

// Get data from specific sensor
sensor_data_t getSensorData(uint64_t macAddress) {
  if (hasSensorData(macAddress)) {
    return sensorDataMap[macAddress];
  }
  // Return empty data if not found
  return {0, 0, 0, 0};
}

// Get last received time for specific sensor
unsigned long getLastReceivedTime(uint64_t macAddress) {
  if (lastReceivedTimeMap.find(macAddress) != lastReceivedTimeMap.end()) {
    return lastReceivedTimeMap[macAddress];
  }
  return 0;
}

// Legacy functions for backward compatibility (use Olga data)
bool hasSensorData() {
  return hasSensorData(OLGA_MAC);
}

sensor_data_t getLatestSensorData() {
  return getSensorData(OLGA_MAC);
}

unsigned long getLastReceivedTime() {
  return getLastReceivedTime(OLGA_MAC);
}

void printSensorData(const sensor_data_t& data, const uint8_t* mac) {
  uint64_t macKey = macToUint64(mac);
  const char* sensorName = getSensorName(macKey);
  Serial.printf("%s: %.2fV (%d%%) | Moisture: %d%%\n",
                sensorName, data.batteryVoltage, data.batteryPercent, data.soilMoisture);
}
