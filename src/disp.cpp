#include "disp.h"
#include "config.h"
#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <Fonts/FreeSansBold18pt7b.h>  // Bold font for large tram time
#include <Fonts/FreeSansBold12pt7b.h>  // Bold font for medium text
#include <Fonts/FreeSans9pt7b.h>       // Regular font for small text

// Pin definitions from config.h
Adafruit_ST7735* tft = nullptr;

// PWM settings for backlight brightness control
#define BACKLIGHT_PWM_CHANNEL 0
#define BACKLIGHT_PWM_FREQ    5000
#define BACKLIGHT_PWM_RES     8  // 8-bit resolution (0-255)

// 'maculata' plant icon, 32x32px
const unsigned char epd_bitmap_maculata [] PROGMEM = {
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xbf, 0xef, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void initDisplay() {
    Serial.println("=== Display Init Start ===");
    Serial.printf("TFT pins - CS:%d DC:%d RST:%d MOSI:%d SCLK:%d BL:%d\n", 
                  TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_BL);
    
    // Setup PWM for backlight brightness control
    Serial.println("Setting up PWM for backlight control...");
    ledcSetup(BACKLIGHT_PWM_CHANNEL, BACKLIGHT_PWM_FREQ, BACKLIGHT_PWM_RES);
    ledcAttachPin(TFT_BL, BACKLIGHT_PWM_CHANNEL);
    
    // Set initial brightness to 60%
    setDisplayBrightness(60);
    delay(10);
    
    // Manual hardware reset
    Serial.println("Performing hardware reset...");
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH);
    delay(10);
    digitalWrite(TFT_RST, LOW);
    delay(20);
    digitalWrite(TFT_RST, HIGH);
    delay(150);
    Serial.println("Hardware reset complete");
    
    Serial.println("Initializing SPI...");
    SPI.begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    SPI.setFrequency(27000000); // Increase to 27MHz for better performance
    Serial.println("SPI initialized at 27MHz");
    delay(100);
    
    Serial.println("Creating Adafruit_ST7735 object...");
    tft = new Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
    Serial.println("Display object created");
    delay(50);
    
    Serial.println("Initializing ST7735 with 160x128 configuration...");
    // Use INITR_BLACKTAB for 160x128 displays (black tab version)
    tft->initR(INITR_BLACKTAB);
    delay(100);
    
    // Disable display inversion for proper black background
    Serial.println("Setting display inversion OFF for black background...");
    tft->invertDisplay(false);
    delay(50);
    
    // Set rotation to 3 (landscape mode - 160 wide x 128 tall)
    Serial.println("Setting rotation to 3 (landscape 160x128)...");
    tft->setRotation(3);
    
    // Clear screen to BLACK
    Serial.println("Clearing screen to BLACK...");
    tft->fillScreen(ST77XX_BLACK);
    delay(50);
    
    Serial.println("=== Display Init Complete ===");
    Serial.printf("Display size: %dx%d\n", tft->width(), tft->height());
}

void showDebugInfo(String msg, int httpCode, int htmlSize, int found) {
    // ALWAYS fill entire screen with black first
    tft->fillScreen(ST77XX_BLACK);
    
    // Show message in white
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(2);
    tft->setCursor(5, 10);
    tft->println(msg);
    
    // Show WiFi and time
    tft->setTextSize(1);
    tft->setCursor(5, 40);
    tft->printf("WiFi: %s", WiFi.isConnected() ? "Connected" : "Disconnected");
    
    tft->setCursor(5, 55);
    time_t now = time(nullptr);
    if (now > 100000) {
        struct tm* ti = localtime(&now);
        tft->printf("Time: %02d:%02d:%02d", ti->tm_hour, ti->tm_min, ti->tm_sec);
    } else {
        tft->print("Time: Syncing...");
    }
    
    // Show HTTP status
    tft->setCursor(5, 70);
    if (httpCode > 0) {
        tft->printf("HTTP: %d %s", httpCode, httpCode == 200 ? "OK" : "ERR");
    } else {
        tft->print("HTTP: Not fetched");
    }
    
    // Show HTML size
    tft->setCursor(5, 85);
    if (htmlSize > 0) {
        tft->printf("Data: %d bytes", htmlSize);
    }
    
    // Show found entries
    tft->setCursor(5, 100);
    if (found >= 0) {
        tft->printf("Found: %d trams", found);
    }
}

void showMessage(String msg) {
    Serial.printf("Display: %s\n", msg.c_str());
    int code = getLastHttpCode();
    int size = getLastHtmlSize();
    int found = getLastFoundEntries();
    showDebugInfo(msg, code, size, found);
}

void showTrams(std::vector<Tram> trams) {
    // Always fill entire screen with black
    tft->fillScreen(ST77XX_BLACK);
    
    if (trams.empty()) {
        showMessage("No trams");
        return;
    }
    
    Serial.printf("Displaying %d trams\n", trams.size());
    
    // Display title at top
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    tft->setCursor(2, 2);
    tft->println(STOP_NAME);
    
    // Draw separator line
    tft->drawFastHLine(0, 12, 160, ST77XX_BLUE);
    
    // Show up to 5 trams (160x128 landscape)
    int y = 18;
    for (size_t i = 0; i < trams.size() && i < 5; i++) {
        // Line number (yellow, left)
        tft->setTextColor(ST77XX_YELLOW);
        tft->setTextSize(2);
        tft->setCursor(2, y);
        tft->print(trams[i].line);
        
        // Destination (white, middle)
        tft->setTextColor(ST77XX_WHITE);
        tft->setTextSize(1);
        tft->setCursor(30, y + 4);
        String d = trams[i].dest;
        if (d.length() > 15) d = d.substring(0, 15);
        tft->print(d);
        
        // Minutes (green, right)
        tft->setTextColor(ST77XX_GREEN);
        tft->setTextSize(2);
        int minsWidth = (trams[i].mins < 10) ? 12 : 24;
        tft->setCursor(160 - minsWidth - 15, y);
        tft->print(trams[i].mins);
        tft->setTextSize(1);
        tft->setCursor(160 - 12, y + 4);
        tft->print("m");
        
        y += 22;
    }
}

// New function to show trams with weather and sensor data
void showTramsWithSensor(std::vector<Tram> trams, const sensor_data_t& sensorData) {
    // Clear screen
    tft->fillScreen(ST77XX_BLACK);
    
    if (trams.empty()) {
        showMessage("No trams");
        return;
    }
    
    // ===== SEPARATOR LINES (GRAY, NO OUTER BORDER) =====
    tft->drawFastHLine(0, 64, 160, COLOR_GRAY);     // Horizontal separator (top | bottom)
    
    // ===== LAYOUT: TOP LEFT = TRAMS (0-80, 0-64) =====
    // Header: "Tram 17" in WHITE
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    tft->setCursor(3, 3);
    tft->print("Tram 17");
    
    // Current time (far right of entire screen)
    time_t now = time(nullptr);
    if (now > 100000) {
        struct tm* ti = localtime(&now);
        tft->setTextColor(ST77XX_WHITE);
        tft->setCursor(125, 3);  // Far right side
        tft->printf("%02d:%02d", ti->tm_hour, ti->tm_min);
    }
    
    // Gray underline across ENTIRE screen (below header)
    tft->drawFastHLine(0, 12, 160, COLOR_GRAY);
    
    // First tram (LARGE RED BOLD with custom font, NO "m" label, CENTERED properly)
    if (trams.size() > 0) {
        tft->setTextColor(ST77XX_RED);
        tft->setFont(&FreeSansBold18pt7b);  // Use bold font
        int firstTime = trams[0].mins;
        
        // Proper centering based on digit count in LEFT section only (0-48 width, leaving space for right times)
        int centerX;
        if (firstTime == 0) {
            centerX = 5;  // "NOW" - left aligned, limited width
        } else if (firstTime < 10) {
            centerX = 15;  // Single digit - more centered
        } else {
            centerX = 8;  // Double digit - slightly left to fit
        }
        
        tft->setCursor(centerX, 45);  // Y is baseline position with custom fonts
        if (firstTime == 0) {
            tft->print("NOW");
        } else {
            tft->printf("%d", firstTime);
        }
        tft->setFont();  // Reset to default font
    }
    
    // Next 3 tram times (smaller, to the right, more spacing from large number)
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    
    if (trams.size() > 1) {
        tft->setCursor(56, 22);
        tft->printf("%dm", trams[1].mins);
    }
    if (trams.size() > 2) {
        tft->setCursor(56, 36);
        tft->printf("%dm", trams[2].mins);
    }
    if (trams.size() > 3) {
        tft->setCursor(56, 50);
        tft->printf("%dm", trams[3].mins);
    }
    
    // ===== LAYOUT: TOP RIGHT = WEATHER (80-160, 0-64) =====
    // Placeholder for weather (will be filled when weather data is available)
    tft->setTextColor(ST77XX_YELLOW);
    tft->setTextSize(1);
    tft->setCursor(95, 25);
    tft->print("Weather");
    tft->setCursor(95, 38);
    tft->print("Loading...");
    
    // ===== LAYOUT: BOTTOM = SENSOR DATA IN 3 EQUAL SECTIONS (0-160, 64-128) =====
    // Section width: 160/3 = ~53 pixels each
    
    // Gray separators between sections
    tft->drawFastVLine(53, 64, 64, COLOR_GRAY);   // After section 1
    tft->drawFastVLine(106, 64, 64, COLOR_GRAY);  // After section 2
    
    // SECTION 1 (0-53): "Olga" sensor
    tft->setTextColor(ST77XX_GREEN);
    tft->setTextSize(1);
    tft->setCursor(10, 73);
    tft->print("Olga");
    
    tft->setTextColor(ST77XX_WHITE);
    tft->setCursor(4, 90);
    tft->print("S:");
    tft->setCursor(20, 90);
    tft->printf("%d%%", sensorData.soilMoisture);
    
    tft->setCursor(4, 105);
    tft->print("B:");
    tft->setCursor(20, 105);
    tft->printf("%d%%", sensorData.batteryPercent);
    
    // SECTION 2 (53-106): Reserved for future sensor "Rose"
    tft->setTextColor(ST77XX_GREEN);
    tft->setTextSize(1);
    tft->setCursor(63, 73);
    tft->print("Rose");
    
    tft->setTextColor(ST77XX_WHITE);
    tft->setCursor(58, 90);
    tft->print("S:");
    tft->setCursor(74, 90);
    tft->print("--");
    
    tft->setCursor(58, 105);
    tft->print("B:");
    tft->setCursor(74, 105);
    tft->print("--");
    
    // SECTION 3 (106-160): Reserved for third sensor
    tft->setTextColor(COLOR_GRAY);
    tft->setTextSize(1);
    tft->setCursor(118, 73);
    tft->print("---");
    tft->setCursor(112, 90);
    tft->print("S:--");
    tft->setCursor(112, 105);
    tft->print("B:--");
    
    Serial.println("Display updated: Trams + Sensor (quadrant layout)");
}

// Enhanced function to show trams with weather AND sensor data from multiple sensors
void showTramsWithWeatherAndSensor(std::vector<Tram> trams, const Weather& weather, const sensor_data_t& olgaData, const sensor_data_t& aeData, bool hasOlga, bool hasAE) {
    // Clear screen
    tft->fillScreen(ST77XX_BLACK);
    
    if (trams.empty()) {
        showMessage("No trams");
        return;
    }
    
    // ===== SEPARATOR LINES (GRAY, NO OUTER BORDER) =====
    tft->drawFastHLine(0, 64, 160, COLOR_GRAY);     // Horizontal separator (top | bottom)
    
    // ===== LAYOUT: TOP LEFT = TRAMS (0-80, 0-64) =====
    // Header: "Tram 17" in WHITE
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    tft->setCursor(3, 3);
    tft->print("Tram 17");
    
    // Current time (far right of entire screen)
    time_t now = time(nullptr);
    if (now > 100000) {
        struct tm* ti = localtime(&now);
        tft->setTextColor(ST77XX_WHITE);
        tft->setCursor(125, 3);  // Far right side
        tft->printf("%02d:%02d", ti->tm_hour, ti->tm_min);
    }
    
    // Gray underline across ENTIRE screen (below header)
    tft->drawFastHLine(0, 12, 160, COLOR_GRAY);
    
    // First tram (LARGE RED BOLD with custom font, NO "m" label, CENTERED properly)
    if (trams.size() > 0) {
        tft->setTextColor(ST77XX_RED);
        tft->setFont(&FreeSansBold18pt7b);  // Use bold font
        int firstTime = trams[0].mins;
        
        // Proper centering based on digit count in LEFT section only (0-48 width, leaving space for right times)
        int centerX;
        if (firstTime == 0) {
            centerX = 2;  // "NOW" - tighter left aligned to avoid overflow
        } else if (firstTime < 10) {
            centerX = 15;  // Single digit - more centered
        } else {
            centerX = 8;  // Double digit - slightly left to fit
        }
        
        tft->setCursor(centerX, 45);  // Y is baseline position with custom fonts
        if (firstTime == 0) {
            tft->print("NOW");
        } else {
            tft->printf("%d", firstTime);
        }
        tft->setFont();  // Reset to default font
    }
    
    // Next 3 tram times (smaller, to the right, more spacing from large number)
    tft->setTextColor(ST77XX_WHITE);
    tft->setTextSize(1);
    
    if (trams.size() > 1) {
        tft->setCursor(56, 22);
        tft->printf("%dm", trams[1].mins);
    }
    if (trams.size() > 2) {
        tft->setCursor(56, 36);
        tft->printf("%dm", trams[2].mins);
    }
    if (trams.size() > 3) {
        tft->setCursor(56, 50);
        tft->printf("%dm", trams[3].mins);
    }
    
    // ===== LAYOUT: TOP RIGHT = WEATHER (80-160, 0-64) =====
    if (weather.valid) {
        // High temperature (red, smaller) - aligned with top of large temp
        tft->setTextColor(ST77XX_RED);
        tft->setTextSize(1);
        tft->setCursor(135, 18);
        tft->print("H:");
        tft->printf("%.0f", weather.tempMax);
        
        // Current temperature (white, with bold font) - sized to fit without touching top line
        tft->setTextColor(ST77XX_WHITE);
        tft->setFont(&FreeSansBold12pt7b);  // 12pt for proper fit (5% smaller than 18pt)
        tft->setCursor(90, 33);  // Baseline positioning
        tft->printf("%.0f", weather.temp);
        tft->setFont();
        tft->setTextSize(1);
        tft->setCursor(122, 26);
        tft->print("C");
        
        // Low temperature (cyan, smaller) - bottom aligned with temp baseline
        tft->setTextColor(ST77XX_CYAN);
        tft->setCursor(135, 33);
        tft->print("L:");
        tft->printf("%.0f", weather.tempMin);
        
        // Wind speed (white) - centered within weather block width, with space before m/s
        tft->setTextColor(ST77XX_WHITE);
        tft->setTextSize(1);
        tft->setCursor(98, 50);
        tft->printf("%.1f ", weather.windSpeed);  // Added space before m/s
        tft->print("m/s");
    } else {
        tft->setTextColor(ST77XX_YELLOW);
        tft->setTextSize(1);
        tft->setCursor(88, 25);
        tft->print("Weather");
        tft->setCursor(88, 37);
        tft->print("Loading");
    }
    
    // ===== LAYOUT: BOTTOM = SENSOR DATA IN 3 EQUAL SECTIONS (0-160, 64-128) =====
    // Section width: 160/3 = ~53 pixels each
    
    // Gray separators between sections
    tft->drawFastVLine(53, 64, 64, COLOR_GRAY);   // After section 1
    tft->drawFastVLine(106, 64, 64, COLOR_GRAY);  // After section 2
    
    // SECTION 1 (0-53): "Olga" sensor
    tft->setTextColor(ST77XX_GREEN);
    tft->setTextSize(1);
    tft->setCursor(10, 73);
    tft->print("Olga");
    
    if (hasOlga) {
        tft->setTextColor(ST77XX_WHITE);
        tft->setCursor(4, 90);
        tft->print("S:");
        tft->setCursor(20, 90);
        tft->printf("%d%%", olgaData.soilMoisture);
        
        tft->setCursor(4, 105);
        tft->print("B:");
        tft->setCursor(20, 105);
        tft->printf("%d%%", olgaData.batteryPercent);
    } else {
        tft->setTextColor(ST77XX_WHITE);
        tft->setCursor(4, 90);
        tft->print("S:");
        tft->setCursor(20, 90);
        tft->print("--");
        
        tft->setCursor(4, 105);
        tft->print("B:");
        tft->setCursor(20, 105);
        tft->print("--");
    }
    
    // SECTION 2 (53-106): "A&E" sensor
    tft->setTextColor(ST77XX_GREEN);
    tft->setTextSize(1);
    tft->setCursor(63, 73);
    tft->print("A&E");
    
    if (hasAE) {
        tft->setTextColor(ST77XX_WHITE);
        tft->setCursor(58, 90);
        tft->print("S:");
        tft->setCursor(74, 90);
        tft->printf("%d%%", aeData.soilMoisture);
        
        tft->setCursor(58, 105);
        tft->print("B:");
        tft->setCursor(74, 105);
        tft->printf("%d%%", aeData.batteryPercent);
    } else {
        tft->setTextColor(ST77XX_WHITE);
        tft->setCursor(58, 90);
        tft->print("S:");
        tft->setCursor(74, 90);
        tft->print("--");
        
        tft->setCursor(58, 105);
        tft->print("B:");
        tft->setCursor(74, 105);
        tft->print("--");
    }
    
    // SECTION 3 (106-160): Reserved for third sensor
    tft->setTextColor(COLOR_GRAY);
    tft->setTextSize(1);
    tft->setCursor(118, 73);
    tft->print("---");
    tft->setCursor(112, 90);
    tft->print("S:--");
    tft->setCursor(112, 105);
    tft->print("B:--");
    
    Serial.println("Display updated: Trams, Weather, and Multi-Sensor Data");
}

// ========== BRIGHTNESS CONTROL ==========

// Set display backlight brightness (0-100%)
void setDisplayBrightness(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    
    // Convert percentage to 8-bit PWM value (0-255)
    int pwmValue = (percent * 255) / 100;
    
    ledcWrite(BACKLIGHT_PWM_CHANNEL, pwmValue);
    Serial.printf("Display brightness set to %d%% (PWM: %d/255)\n", percent, pwmValue);
}

// Update brightness based on current time (60% day, 20% night)
void updateBrightnessForTime() {
    time_t now = time(nullptr);
    if (now < 100000) {
        Serial.println("Time not synced yet, keeping default brightness");
        return;
    }
    
    struct tm* ti = localtime(&now);
    int hour = ti->tm_hour;
    
    // Night mode: 00:00 to 06:00 (midnight to 6am) = 20% brightness
    // Day mode: 06:00 to 00:00 (6am to midnight) = 60% brightness
    if (hour >= 0 && hour < 6) {
        setDisplayBrightness(20);
        Serial.printf("Night mode active (%02d:%02d) - 20%% brightness\n", hour, ti->tm_min);
    } else {
        setDisplayBrightness(60);
        Serial.printf("Day mode active (%02d:%02d) - 60%% brightness\n", hour, ti->tm_min);
    }
}
