#include "ov_api.h"
#include <HTTPClient.h>
#include <time.h>

OVApi::OVApi(const String& baseUrl, const String& stopCode) 
    : baseUrl(baseUrl), stopAreaCode(stopCode) {
}

String OVApi::makeHttpRequest(const String& url) {
    HTTPClient http;
    String payload = "";
    
    http.begin(url);
    http.setTimeout(10000); // 10 second timeout
    
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        Serial.println("HTTP request successful");
    } else {
        Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return payload;
}

bool OVApi::fetchDepartures() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        return false;
    }
    
    String url = baseUrl + "/stopareacode/" + stopAreaCode;
    Serial.println("Fetching departures from: " + url);
    
    String response = makeHttpRequest(url);
    
    if (response.length() == 0) {
        Serial.println("Empty response from API");
        return false;
    }
    
    return parseJsonResponse(response);
}

bool OVApi::parseJsonResponse(const String& json) {
    // Clear previous departures
    departures.clear();
    
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, json);
    
    if (error) {
        Serial.print("JSON parsing failed: ");
        Serial.println(error.c_str());
        return false;
    }
    
    // Navigate through the JSON structure
    JsonObject root = doc.as<JsonObject>();
    
    // Iterate through stop areas
    for (JsonPair stopArea : root) {
        JsonObject stopData = stopArea.value().as<JsonObject>();
        
        // Check if this is the correct stop area
        if (stopData.containsKey("Stop")) {
            JsonObject stop = stopData["Stop"];
            String stopAreaCodeCheck = stop["StopAreaCode"].as<String>();
            
            if (stopAreaCodeCheck != stopAreaCode) {
                continue;
            }
        }
        
        // Get passes (departures)
        if (stopData.containsKey("Passes")) {
            JsonObject passes = stopData["Passes"];
            
            for (JsonPair pass : passes) {
                JsonObject passData = pass.value().as<JsonObject>();
                
                Departure dep;
                dep.lineName = passData["LinePublicNumber"].as<String>();
                dep.destination = passData["DestinationName50"].as<String>();
                
                // Calculate time until departure
                String expectedDepartureTime = passData["ExpectedDepartureTime"].as<String>();
                String targetDepartureTime = passData["TargetDepartureTime"].as<String>();
                
                // Check if it's real-time or scheduled
                dep.isRealTime = !expectedDepartureTime.isEmpty();
                
                // Parse time and calculate minutes
                String timeToUse = dep.isRealTime ? expectedDepartureTime : targetDepartureTime;
                
                if (!timeToUse.isEmpty()) {
                    // Time format: "2024-12-03T15:30:00"
                    int year, month, day, hour, minute, second;
                    sscanf(timeToUse.c_str(), "%d-%d-%dT%d:%d:%d", 
                           &year, &month, &day, &hour, &minute, &second);
                    
                    // Get current time
                    time_t now;
                    time(&now);
                    struct tm* currentTime = localtime(&now);
                    
                    // Calculate minutes until departure
                    struct tm departureTime = {0};
                    departureTime.tm_year = year - 1900;
                    departureTime.tm_mon = month - 1;
                    departureTime.tm_mday = day;
                    departureTime.tm_hour = hour;
                    departureTime.tm_min = minute;
                    departureTime.tm_sec = second;
                    
                    time_t depTime = mktime(&departureTime);
                    int diffMinutes = (depTime - now) / 60;
                    
                    dep.departureTime = diffMinutes;
                    
                    // Format time string
                    char timeStr[6];
                    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", hour, minute);
                    dep.departureTimeStr = String(timeStr);
                    
                    // Only add future departures
                    if (diffMinutes >= 0) {
                        departures.push_back(dep);
                    }
                }
            }
        }
    }
    
    // Sort departures by time
    std::sort(departures.begin(), departures.end(), 
              [](const Departure& a, const Departure& b) {
                  return a.departureTime < b.departureTime;
              });
    
    Serial.printf("Parsed %d departures\n", departures.size());
    return departures.size() > 0;
}

std::vector<Departure> OVApi::getDepartures(int maxCount) {
    if (maxCount > 0 && departures.size() > maxCount) {
        return std::vector<Departure>(departures.begin(), departures.begin() + maxCount);
    }
    return departures;
}

int OVApi::getDepartureCount() {
    return departures.size();
}

void OVApi::printDepartures() {
    Serial.println("=== Departures ===");
    for (size_t i = 0; i < departures.size() && i < 10; i++) {
        Departure& dep = departures[i];
        Serial.printf("Line %s to %s: %d min (%s) %s\n", 
                      dep.lineName.c_str(), 
                      dep.destination.c_str(), 
                      dep.departureTime,
                      dep.departureTimeStr.c_str(),
                      dep.isRealTime ? "[RT]" : "[SCH]");
    }
}
