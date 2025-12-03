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
    String stopAreaCode;
    std::vector<Departure> departures;

    bool parseJsonResponse(const String& json);
    String makeHttpRequest(const String& url);

public:
    OVApi(const String& baseUrl, const String& stopCode);
    bool fetchDepartures();
    std::vector<Departure> getDepartures(int maxCount = 10);
    int getDepartureCount();
    void printDepartures();
};

#endif // OV_API_H
