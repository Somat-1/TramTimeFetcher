#ifndef WEATHER_H
#define WEATHER_H

#include <Arduino.h>

struct Weather {
  float temp;
  float tempMin;
  float tempMax;
  float windSpeed;  // in m/s
  String description;
  bool valid;
};

bool fetchWeather(Weather& weather);

#endif
