#include "weather.h"
#include "config.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

bool fetchWeather(Weather& weather) {
  HTTPClient http;
  
  // Open-Meteo API - no API key needed!
  String url = "https://api.open-meteo.com/v1/forecast?latitude=" + 
               String(WEATHER_LAT) + "&longitude=" + String(WEATHER_LON) + 
               "&current=temperature_2m,wind_speed_10m&daily=temperature_2m_max,temperature_2m_min&timezone=Europe%2FAmsterdam";
  
  Serial.println("Fetching weather from Open-Meteo...");
  http.begin(url);
  
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Weather data received");
    Serial.println("Payload length: " + String(payload.length()));
    
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (error) {
      Serial.print("JSON parsing failed: ");
      Serial.println(error.c_str());
      http.end();
      return false;
    }
    
    // Get current temperature and wind speed
    weather.temp = doc["current"]["temperature_2m"];
    float windSpeedKmh = doc["current"]["wind_speed_10m"];
    weather.windSpeed = windSpeedKmh / 3.6;  // Convert km/h to m/s
    
    // Get today's min/max from daily forecast
    weather.tempMin = doc["daily"]["temperature_2m_min"][0];
    weather.tempMax = doc["daily"]["temperature_2m_max"][0];
    
    weather.description = "Clear";
    weather.valid = true;
    
    Serial.printf("Weather: %.1f°C (%.1f-%.1f), Wind: %.1f m/s\n", 
                  weather.temp, weather.tempMin, weather.tempMax, weather.windSpeed);
    
    http.end();
    return true;
  } else {
    Serial.printf("Weather fetch failed: %d\n", httpCode);
    http.end();
    return false;
  }
}
