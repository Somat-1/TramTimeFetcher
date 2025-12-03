#include "display_manager.h"
#include "config.h"

// Colors
#define COLOR_BACKGROUND TFT_BLACK
#define COLOR_HEADER TFT_ORANGE
#define COLOR_TEXT TFT_WHITE
#define COLOR_LINE_NUMBER TFT_YELLOW
#define COLOR_TIME TFT_GREEN
#define COLOR_REALTIME TFT_CYAN
#define COLOR_SCHEDULED TFT_LIGHTGREY

DisplayManager::DisplayManager() : tft(TFT_eSPI()), needsRefresh(true) {
    screenWidth = 240;
    screenHeight = 135;
}

void DisplayManager::begin() {
    tft.init();
    tft.setRotation(1); // Landscape mode
    tft.fillScreen(COLOR_BACKGROUND);
    tft.setTextColor(COLOR_TEXT, COLOR_BACKGROUND);
}

void DisplayManager::clear() {
    tft.fillScreen(COLOR_BACKGROUND);
    needsRefresh = true;
}

void DisplayManager::setBrightness(uint8_t brightness) {
    // Note: TFT_eSPI doesn't have built-in brightness control
    // You may need to control via hardware PWM on backlight pin
    // This is a placeholder
}

void DisplayManager::showWelcomeScreen() {
    clear();
    tft.setTextSize(2);
    tft.setTextColor(COLOR_HEADER);
    tft.setCursor(30, 40);
    tft.println("Tram Times");
    
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(60, 70);
    tft.println("Statenlaan");
    tft.setCursor(50, 85);
    tft.println("Den Haag");
}

void DisplayManager::showConnecting() {
    clear();
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TEXT);
    tft.setCursor(50, 60);
    tft.println("Connecting to WiFi...");
}

void DisplayManager::showConnected() {
    clear();
    tft.setTextSize(1);
    tft.setTextColor(COLOR_TIME);
    tft.setCursor(60, 60);
    tft.println("WiFi Connected!");
    delay(1000);
}

void DisplayManager::showUpdating() {
    // Show small indicator in corner
    tft.fillCircle(230, 10, 5, COLOR_HEADER);
}

void DisplayManager::showError(const String& message) {
    clear();
    tft.setTextSize(1);
    tft.setTextColor(TFT_RED);
    tft.setCursor(10, 60);
    tft.println("Error:");
    tft.setCursor(10, 75);
    tft.println(message);
}

void DisplayManager::showDepartures(const std::vector<Departure>& departures) {
    clear();
    
    // Header
    tft.fillRect(0, 0, screenWidth, 20, COLOR_HEADER);
    tft.setTextSize(1);
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(5, 6);
    tft.println("Statenlaan - Den Haag");
    
    if (departures.empty()) {
        tft.setTextColor(COLOR_TEXT);
        tft.setTextSize(1);
        tft.setCursor(40, 60);
        tft.println("No departures found");
        return;
    }
    
    // Display departures
    int y = 25;
    int lineHeight = 14;
    int maxDisplay = min((int)departures.size(), MAX_DEPARTURES_DISPLAY);
    
    for (int i = 0; i < maxDisplay; i++) {
        const Departure& dep = departures[i];
        
        // Line number (bold, yellow)
        tft.setTextColor(COLOR_LINE_NUMBER);
        tft.setTextSize(1);
        tft.setCursor(5, y);
        tft.printf("%3s", dep.lineName.c_str());
        
        // Destination (white, truncated if needed)
        tft.setTextColor(COLOR_TEXT);
        tft.setCursor(35, y);
        String dest = dep.destination;
        if (dest.length() > 15) {
            dest = dest.substring(0, 15);
        }
        tft.print(dest);
        
        // Time until departure (green for soon, cyan for later)
        if (dep.departureTime < 5) {
            tft.setTextColor(TFT_RED);
        } else if (dep.departureTime < 10) {
            tft.setTextColor(COLOR_TIME);
        } else {
            tft.setTextColor(COLOR_REALTIME);
        }
        
        tft.setCursor(170, y);
        if (dep.departureTime == 0) {
            tft.print("NOW");
        } else if (dep.departureTime < 60) {
            tft.printf("%2d min", dep.departureTime);
        } else {
            tft.print(dep.departureTimeStr);
        }
        
        // Real-time indicator
        if (!dep.isRealTime) {
            tft.setTextColor(COLOR_SCHEDULED);
            tft.setCursor(220, y);
            tft.print("*");
        }
        
        y += lineHeight;
        
        // Draw separator line
        if (i < maxDisplay - 1) {
            tft.drawLine(5, y - 2, screenWidth - 5, y - 2, TFT_DARKGREY);
        }
    }
    
    // Footer - legend
    y = screenHeight - 10;
    tft.setTextSize(1);
    tft.setTextColor(COLOR_SCHEDULED);
    tft.setCursor(5, y);
    tft.print("* = Scheduled");
}
