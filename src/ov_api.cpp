#include "ov_api.h"
#include "config.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <time.h>

OVApi::OVApi(const String& baseUrl, const String& stopCode) 
    : baseUrl(baseUrl), stopCode(stopCode) {
}

String OVApi::makeHttpRequest(const String& url) {
    HTTPClient http;
    WiFiClientSecure client;
    String payload = "";
    
    // Don't verify SSL certificate (for simplicity on ESP32)
    client.setInsecure();
    
    http.begin(client, url);
    http.setTimeout(15000); // 15 second timeout
    http.addHeader("User-Agent", "ESP32-TramReader/1.0");
    
    Serial.println("Making HTTP request to: " + url);
    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
        payload = http.getString();
        Serial.printf("Received %d bytes\n", payload.length());
    } else {
        Serial.printf("HTTP request failed, code: %d, error: %s\n", 
                     httpCode, http.errorToString(httpCode).c_str());
    }
    
    http.end();
    return payload;
}

String OVApi::extractText(const String& html, const String& startTag, const String& endTag) {
    int startIdx = html.indexOf(startTag);
    if (startIdx == -1) return "";
    
    startIdx += startTag.length();
    int endIdx = html.indexOf(endTag, startIdx);
    if (endIdx == -1) return "";
    
    String result = html.substring(startIdx, endIdx);
    result.trim();
    return result;
}

bool OVApi::fetchDepartures() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        return false;
    }
    
    String url = baseUrl + stopCode;
    Serial.println("Fetching departures from DRGL: " + url);
    
    String response = makeHttpRequest(url);
    
    if (response.length() == 0) {
        Serial.println("Empty response from DRGL");
        return false;
    }
    
    return parseHtmlResponse(response);
}

bool OVApi::parseHtmlResponse(const String& html) {
    // Clear previous departures
    departures.clear();
    
    Serial.println("Parsing HTML response...");
    
    // Get current time
    time_t now;
    time(&now);
    struct tm* currentTime = localtime(&now);
    int currentMinutes = currentTime->tm_hour * 60 + currentTime->tm_min;
    
    // Find all departure entries
    int searchPos = 0;
    int departureCount = 0;
    
    while (searchPos < html.length() && departureCount < 20) {
        // Find departure time
        int timeStart = html.indexOf("ott-departure-time", searchPos);
        if (timeStart == -1) break;
        
        // Extract time (format: HH:MM)
        int timeTagEnd = html.indexOf('>', timeStart);
        int timeEnd = html.indexOf('<', timeTagEnd);
        String timeStr = html.substring(timeTagEnd + 1, timeEnd);
        timeStr.trim();
        
        // Find line number
        int lineStart = html.indexOf("ott-linecode", timeEnd);
        if (lineStart == -1) break;
        
        int lineTagEnd = html.indexOf('>', lineStart);
        int lineEnd = html.indexOf('<', lineTagEnd);
        String lineNum = html.substring(lineTagEnd + 1, lineEnd);
        lineNum.trim();
        
        // Find destination
        int destStart = html.indexOf("ott-destination", lineEnd);
        if (destStart == -1) break;
        
        int destTagEnd = html.indexOf('>', destStart);
        int destEnd = html.indexOf('<', destTagEnd);
        String destination = html.substring(destTagEnd + 1, destEnd);
        destination.trim();
        
        // Parse time
        int hour, minute;
        if (sscanf(timeStr.c_str(), "%d:%d", &hour, &minute) == 2) {
            // Calculate minutes until departure
            int depMinutes = hour * 60 + minute;
            int diffMinutes = depMinutes - currentMinutes;
            
            // Handle day rollover
            if (diffMinutes < -60) {
                diffMinutes += 24 * 60;
            }
            
            // Apply time adjustment (subtract 1 minute to show earlier)
            #ifdef TIME_ADJUSTMENT_MINUTES
            diffMinutes += TIME_ADJUSTMENT_MINUTES;
            #endif
            
            // Only add future departures (within next 2 hours)
            if (diffMinutes >= 0 && diffMinutes < 120) {
                Departure dep;
                dep.lineName = lineNum;
                dep.destination = destination;
                dep.departureTime = diffMinutes;
                dep.departureTimeStr = timeStr;
                dep.isRealTime = true; // DRGL shows real-time data
                
                departures.push_back(dep);
                departureCount++;
                
                Serial.printf("  Found: Line %s to %s at %s (%d min adjusted)\n", 
                             lineNum.c_str(), destination.c_str(), 
                             timeStr.c_str(), diffMinutes);
            }
        }
        
        searchPos = destEnd + 1;
    }
    
    Serial.printf("Parsed %d departures\n", departures.size());
    return departures.size() > 0;
}

std::vector<Departure> OVApi::getDepartures(int maxCount) {
    if (maxCount > 0 && departures.size() > (size_t)maxCount) {
        return std::vector<Departure>(departures.begin(), departures.begin() + maxCount);
    }
    return departures;
}

int OVApi::getDepartureCount() {
    return departures.size();
}

void OVApi::printDepartures() {
    Serial.println("=== Tram Departures ===");
    for (size_t i = 0; i < departures.size() && i < 10; i++) {
        Departure& dep = departures[i];
        Serial.printf("Line %s to %s: %d min (%s)\n", 
                      dep.lineName.c_str(), 
                      dep.destination.c_str(), 
                      dep.departureTime,
                      dep.departureTimeStr.c_str());
    }
}
