#include "api.h"
#include "config.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include <time.h>

// Global variables for debugging
int lastHttpCode = 0;
int lastHtmlSize = 0;
int lastFoundEntries = 0;

// Helper functions
int getLastHttpCode() { return lastHttpCode; }
int getLastHtmlSize() { return lastHtmlSize; }
int getLastFoundEntries() { return lastFoundEntries; }

// Find the next occurrence of a substring, case-insensitive
int findStrNoCase(const String& haystack, const char* needle, int startPos = 0) {
    int needleLen = strlen(needle);
    int haystackLen = haystack.length();
    
    for (int i = startPos; i <= haystackLen - needleLen; i++) {
        bool match = true;
        for (int j = 0; j < needleLen; j++) {
            if (tolower(haystack[i + j]) != tolower(needle[j])) {
                match = false;
                break;
            }
        }
        if (match) return i;
    }
    return -1;
}

// Extract text between two tags
String extractBetween(const String& html, const char* startTag, const char* endTag, int& pos) {
    int start = findStrNoCase(html, startTag, pos);
    if (start == -1) return "";
    
    start += strlen(startTag);
    int end = findStrNoCase(html, endTag, start);
    if (end == -1) return "";
    
    pos = end + strlen(endTag);
    return html.substring(start, end);
}

// Parse time string like "12:34" and return minutes from now
int parseTimeToMinutes(const String& timeStr) {
    if (timeStr.length() < 5) return -1;
    
    int hour = timeStr.substring(0, 2).toInt();
    int minute = timeStr.substring(3, 5).toInt();
    
    time_t now = time(nullptr);
    if (now < 100000) return -1;
    
    struct tm* ti = localtime(&now);
    int currentHour = ti->tm_hour;
    int currentMinute = ti->tm_min;
    
    // Calculate minutes until departure
    int departMinutes = hour * 60 + minute;
    int currentMinutes = currentHour * 60 + currentMinute;
    int diff = departMinutes - currentMinutes;
    
    // Handle midnight crossing
    if (diff < -720) diff += 1440;  // Next day
    if (diff > 720) diff -= 1440;    // Previous day
    
    return diff;
}

std::vector<Tram> fetchTrams() {
    std::vector<Tram> trams;
    lastHttpCode = 0;
    lastHtmlSize = 0;
    lastFoundEntries = 0;
    
    if (!WiFi.isConnected()) {
        Serial.println("ERROR: WiFi not connected!");
        return trams;
    }
    
    String url = "https://drgl.nl/stop/" + String(STOP_CODE);
    Serial.printf("Fetching: %s\n", url.c_str());
    
    HTTPClient http;
    http.setTimeout(10000);  // 10 second timeout
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    
    if (!http.begin(url)) {
        Serial.println("ERROR: Failed to begin HTTP connection");
        return trams;
    }
    
    // Set user agent to avoid blocking
    http.addHeader("User-Agent", "Mozilla/5.0 (ESP32)");
    
    Serial.println("Sending HTTP GET request...");
    lastHttpCode = http.GET();
    Serial.printf("HTTP Response Code: %d\n", lastHttpCode);
    
    if (lastHttpCode != 200) {
        Serial.printf("ERROR: HTTP failed with code %d\n", lastHttpCode);
        http.end();
        return trams;
    }
    
    String html = http.getString();
    lastHtmlSize = html.length();
    http.end();
    
    Serial.printf("Received %d bytes of HTML\n", lastHtmlSize);
    
    if (lastHtmlSize < 100) {
        Serial.println("ERROR: HTML response too small");
        return trams;
    }
    
    // Simple text-based parsing for DRGL
    // Look for simple text patterns like: "14:40 17 Wateringen"
    // The page shows plain text in format: HH:MM [line] [destination]
    
    Serial.println("Parsing HTML for tram departures...");
    Serial.println("First 500 chars of HTML:");
    Serial.println(html.substring(0, 500));
    
    int pos = 0;
    int foundCount = 0;
    
    // Look for time patterns HH:MM followed by a number (line) and text (destination)
    while (pos < html.length() && trams.size() < 10) {
        // Find time pattern HH:MM
        bool foundTime = false;
        String timeStr = "";
        
        for (int i = pos; i < html.length() - 4; i++) {
            char c1 = html[i];
            char c2 = html[i+1];
            char c3 = html[i+2];
            char c4 = html[i+3];
            char c5 = html[i+4];
            
            // Check for HH:MM pattern (digit digit : digit digit)
            if (isdigit(c1) && isdigit(c2) && c3 == ':' && isdigit(c4) && isdigit(c5)) {
                timeStr = String(c1) + String(c2) + ":" + String(c4) + String(c5);
                pos = i + 5;
                foundTime = true;
                foundCount++;
                break;
            }
        }
        
        if (!foundTime) break;
        
        // Now extract line number and destination after the time
        // Skip whitespace and HTML tags
        while (pos < html.length() && (html[pos] == ' ' || html[pos] == '\t' || 
                                        html[pos] == '\n' || html[pos] == '\r' || 
                                        html[pos] == '<')) {
            if (html[pos] == '<') {
                // Skip entire tag
                while (pos < html.length() && html[pos] != '>') pos++;
                pos++;
            } else {
                pos++;
            }
        }
        
        // Extract line number (should be 1-3 digits)
        String lineNum = "";
        while (pos < html.length() && isdigit(html[pos])) {
            lineNum += html[pos];
            pos++;
        }
        
        // Skip whitespace and tags again
        while (pos < html.length() && (html[pos] == ' ' || html[pos] == '\t' || 
                                        html[pos] == '\n' || html[pos] == '\r' || 
                                        html[pos] == '<')) {
            if (html[pos] == '<') {
                while (pos < html.length() && html[pos] != '>') pos++;
                pos++;
            } else {
                pos++;
            }
        }
        
        // Extract destination (until we hit < or newline)
        String dest = "";
        while (pos < html.length() && html[pos] != '<' && html[pos] != '\n' && 
               html[pos] != '\r' && dest.length() < 50) {
            dest += html[pos];
            pos++;
        }
        
        dest.trim();
        
        // Calculate minutes from now
        int mins = parseTimeToMinutes(timeStr);
        
        Serial.printf("Found #%d: Time=%s Line=%s Dest=%s (%d min)\n", 
                     foundCount, timeStr.c_str(), lineNum.c_str(), 
                     dest.c_str(), mins);
        
        // Only add if within next 60 minutes and has valid line number
        if (mins >= 0 && mins <= 60 && lineNum.length() > 0) {
            Tram t;
            t.line = lineNum;
            t.dest = dest.length() > 0 ? dest : "Unknown";
            t.mins = mins;
            trams.push_back(t);
        }
    }
    
    lastFoundEntries = trams.size();
    Serial.printf("Total departures within 60 min: %d\n", lastFoundEntries);
    
    return trams;
}
