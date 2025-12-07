#ifndef ESPNOW_RECEIVER_H
#define ESPNOW_RECEIVER_H

#include <Arduino.h>
#include <map>

// Data structure matching the soil moisture sensor
typedef struct sensor_data_t {
  float batteryVoltage;
  int batteryPercent;
  int soilMoisture;
  unsigned long timestamp;
} sensor_data_t;

// Known sensor MAC addresses
const uint64_t OLGA_MAC = 0x80F1B2502974;   // Olga sensor
const uint64_t AE_MAC   = 0x80F1B2502948;   // A&E sensor

// Helper to convert MAC array to uint64_t for map key
uint64_t macToUint64(const uint8_t* mac);

// Functions
void initESPNowReceiver();
bool hasSensorData(uint64_t macAddress);
sensor_data_t getSensorData(uint64_t macAddress);
unsigned long getLastReceivedTime(uint64_t macAddress);
void printSensorData(const sensor_data_t& data, const uint8_t* mac);

// Legacy functions for backward compatibility (use Olga data)
bool hasSensorData();
sensor_data_t getLatestSensorData();
unsigned long getLastReceivedTime();

#endif