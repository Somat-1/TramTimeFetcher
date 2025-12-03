#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <TFT_eSPI.h>
#include "ov_api.h"
#include <vector>

class DisplayManager {
private:
    TFT_eSPI tft;
    int screenWidth;
    int screenHeight;
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
    void setBrightness(uint8_t brightness);
};

#endif // DISPLAY_MANAGER_H
