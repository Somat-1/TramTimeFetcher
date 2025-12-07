#ifndef DISP_H
#define DISP_H
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <vector>
#include "api.h"
#include "espnow_receiver.h"
#include "weather.h"

extern Adafruit_ST7735* tft;

void initDisplay();
void showMessage(String msg);
void showDebugInfo(String msg, int httpCode, int htmlSize, int found);
void showTrams(std::vector<Tram> trams);
void showTramsWithSensor(std::vector<Tram> trams, const sensor_data_t& sensorData);
void showTramsWithWeatherAndSensor(std::vector<Tram> trams, const Weather& weather, const sensor_data_t& olgaData, const sensor_data_t& aeData, bool hasOlga, bool hasAE);
void setDisplayBrightness(int percent);
void updateBrightnessForTime();

#endif
