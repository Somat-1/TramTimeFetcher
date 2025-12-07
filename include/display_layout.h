// Display Layout Guide
// 160x128 TFT Display (Landscape)
//
// ┌─────────────────────────────────────┐
// │  TRAMS (0-80)    │  WEATHER (80-160)│
// │                  │                  │
// │  [5]  10  15  20 │  18°H 10°L      │ 0-64
// │                  │  Wind: 4 m/s     │
// ├──────────────────┴──────────────────┤
// │          SENSOR DATA                │ 64-128
// │  "Olga" Bat: 95%  Jack: Soil: 45%  │
// └─────────────────────────────────────┘

#ifndef DISPLAY_LAYOUT_H
#define DISPLAY_LAYOUT_H

// Display dimensions
#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 128

// Layout sections
#define TRAM_SECTION_X 0
#define TRAM_SECTION_Y 0
#define TRAM_SECTION_WIDTH 80
#define TRAM_SECTION_HEIGHT 64

#define WEATHER_SECTION_X 80
#define WEATHER_SECTION_Y 0
#define WEATHER_SECTION_WIDTH 80
#define WEATHER_SECTION_HEIGHT 64

#define SENSOR_SECTION_X 0
#define SENSOR_SECTION_Y 64
#define SENSOR_SECTION_WIDTH 160
#define SENSOR_SECTION_HEIGHT 64

#endif
