// filepath: /Users/tomasvalentinas/Documents/PlatformIO/Projects/TramReader/src/display_manager.cpp
#include "display_manager.h"
#include "config.h"

DisplayManager::DisplayManager() : 
    display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
    currentPage(0),
    totalPages(1),
    needsRefresh(true) {
}

void DisplayManager::begin() {
    pinMode(TOUCH_PIN, INPUT);
    
    Wire.begin(SDA_PIN, SCL_PIN);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;); // Don't proceed, loop forever
    }
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
}

void DisplayManager::clear() {
    display.clearDisplay();
    needsRefresh = true;
}

void DisplayManager::showWelcomeScreen() {
    clear();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.println(F("Tram"));
    display.println(F("Times"));
    
    display.setTextSize(1);
    display.setCursor(10, 45);
    display.println(F("Statenlaan"));
    display.setCursor(10, 55);
    display.println(F("Den Haag"));
    display.display();
}

void DisplayManager::showConnecting() {
    clear();
    display.setTextSize(1);
    display.setCursor(0, 20);
    display.println(F("Connecting"));
    display.println(F("to WiFi..."));
    display.display();
}

void DisplayManager::showConnected() {
    clear();
    display.setTextSize(1);
    display.setCursor(0, 25);
    display.println(F("WiFi"));
    display.println(F("Connected!"));
    display.display();
    delay(1500);
}

void DisplayManager::showUpdating() {
    // Show small dot in corner to indicate updating
    display.fillRect(SCREEN_WIDTH - 5, 0, 5, 5, SSD1306_WHITE);
    display.display();
}

void DisplayManager::showError(const String& message) {
    clear();
    display.setTextSize(1);
    display.setCursor(0, 10);
    display.println(F("Error:"));
    display.setCursor(0, 25);
    
    // Word wrap for small screen
    int lineWidth = 21; // Characters per line
    int startIdx = 0;
    int y = 25;
    
    while (startIdx < message.length() && y < SCREEN_HEIGHT - 8) {
        int endIdx = startIdx + lineWidth;
        if (endIdx > message.length()) {
            endIdx = message.length();
        }
        
        String line = message.substring(startIdx, endIdx);
        display.setCursor(0, y);
        display.println(line);
        
        startIdx = endIdx;
        y += 8;
    }
    
    display.display();
}

void DisplayManager::showDepartures(const std::vector<Departure>& departures) {
    clear();
    
    // Header - small font
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("Statenlaan"));
    display.drawLine(0, 9, SCREEN_WIDTH, 9, SSD1306_WHITE);
    
    if (departures.empty()) {
        display.setCursor(0, 25);
        display.println(F("No departures"));
        display.display();
        return;
    }
    
    // Display up to 4 departures (limited by small screen)
    int y = 13;
    int lineHeight = 13;
    int maxDisplay = min((int)departures.size(), MAX_DEPARTURES_DISPLAY);
    
    for (int i = 0; i < maxDisplay; i++) {
        const Departure& dep = departures[i];
        
        display.setTextSize(1);
        
        // Line number (3 chars)
        display.setCursor(0, y);
        String lineNum = dep.lineName;
        if (lineNum.length() > 3) lineNum = lineNum.substring(0, 3);
        display.print(lineNum);
        
        // Destination (truncated to fit)
        display.setCursor(20, y);
        String dest = dep.destination;
        if (dest.length() > 10) {
            dest = dest.substring(0, 10);
        }
        display.print(dest);
        
        // Time
        display.setCursor(90, y);
        if (dep.departureTime == 0) {
            display.print(F("NOW"));
        } else if (dep.departureTime < 60) {
            display.print(dep.departureTime);
            display.print(F("m"));
        } else {
            // Show HH:MM format
            String timeStr = dep.departureTimeStr;
            if (timeStr.length() > 5) {
                timeStr = timeStr.substring(0, 5);
            }
            display.print(timeStr);
        }
        
        // Real-time indicator
        if (!dep.isRealTime) {
            display.setCursor(120, y);
            display.print(F("*"));
        }
        
        y += lineHeight;
    }
    
    // Show page indicator if more departures
    if (departures.size() > MAX_DEPARTURES_DISPLAY) {
        display.setTextSize(1);
        display.setCursor(0, SCREEN_HEIGHT - 8);
        display.print(F("Touch:more"));
    }
    
    display.display();
}

bool DisplayManager::isTouched() {
    static bool lastState = false;
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 200; // 200ms debounce
    
    bool currentState = digitalRead(TOUCH_PIN);
    
    // Check if state changed
    if (currentState != lastState) {
        lastDebounceTime = millis();
    }
    
    bool result = false;
    if ((millis() - lastDebounceTime) > debounceDelay) {
        // If the button state has been stable for debounce delay
        if (currentState == HIGH && lastState == LOW) {
            result = true;
        }
    }
    
    lastState = currentState;
    return result;
}
