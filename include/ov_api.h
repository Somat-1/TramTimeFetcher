#ifndef OV_API_H
#define OV_API_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>

struct Departure {
    String lineName;           // Line number (e.g., "2", "3", "4")
    String destination;        // Destination name
    int departureTime;         // Minutes until departure
    String departureTimeStr;   // Formatted time (HH:MM)
    bool isRealTime;          // Whether time is real-time or scheduled
};

class OVApi {
private:
    String baseUrl;
    String stopCode;
    std::vector<Departure> departures;

    bool parseHtmlResponse(const String& html);
    String makeHttpRequest(const String& url);
    String extractText(const String& html, const String& startTag, const String& endTag);

public:
    OVApi(const String& baseUrl, const String& stopCode);
    bool fetchDepartures();
    std::vector<Departure> getDepartures(int maxCount = 10);
    int getDepartureCount();
    void printDepartures();
};

#endif // OV_API_H
