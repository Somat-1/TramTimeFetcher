#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "ov_api.h"
#include <vector>

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    int currentPage;
    int totalPages;
    bool needsRefresh;

public:
    DisplayManager();
    void begin();
    void showWelcomeScreen();
    void showConnecting();
    void showConnected();
    void showDepartures(const std::vector<Departure>& departures);
    void showError(const String& message);
    void showUpdating();
    void clear();
    bool isTouched();
    void turnOn();
    void turnOff();
    bool isDisplayOn();
};

#endif // DISPLAY_MANAGER_H
