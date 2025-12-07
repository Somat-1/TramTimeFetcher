# Custom Fonts Guide for Adafruit GFX

## Built-in Fonts (Already Available!)

Adafruit GFX includes these fonts in the library:

### Sans Serif Fonts
- `FreeSans9pt7b` - Regular, 9 point
- `FreeSans12pt7b` - Regular, 12 point  
- `FreeSans18pt7b` - Regular, 18 point
- `FreeSans24pt7b` - Regular, 24 point
- `FreeSansBold9pt7b` - Bold, 9 point
- `FreeSansBold12pt7b` - Bold, 12 point
- `FreeSansBold18pt7b` - Bold, 18 point
- `FreeSansBold24pt7b` - Bold, 24 point

### Serif Fonts
- `FreeSerif9pt7b`, `FreeSerif12pt7b`, etc.

### Monospace Fonts
- `FreeMono9pt7b`, `FreeMono12pt7b`, etc.

## How to Use Built-in Fonts

```cpp
#include <Fonts/FreeSansBold12pt7b.h>  // Include the font you want

// In your display code:
tft->setFont(&FreeSansBold12pt7b);  // Set the font
tft->println("Hello!");
tft->setFont();  // Return to default font (or setFont(NULL))
```

## Using Fonts in TramReader

To use FreeSansBold for the first tram time:

```cpp
#include <Fonts/FreeSansBold18pt7b.h>

// In disp.cpp, for the first tram time:
tft->setFont(&FreeSansBold18pt7b);
tft->setCursor(5, 35);  // Y position needs adjustment with custom fonts
tft->print("15");
tft->setFont();  // Reset to default
```

**Important Notes:**
- Custom fonts use **baseline** positioning (not top-left)
- Y coordinate is the **bottom** of the text, not top
- Text will be drawn **above** the cursor Y position
- You may need to adjust Y positions by +10-20 pixels

## Creating Your Own Fonts (Advanced)

### Method 1: Using fontconvert (included with Adafruit GFX)

1. **Find the fontconvert tool:**
   ```bash
   ~/.platformio/packages/framework-arduinoespressif32/libraries/Adafruit_GFX_Library/fontconvert
   ```

2. **Convert a TrueType font:**
   ```bash
   cd /path/to/your/font
   fontconvert YourFont.ttf 18 32 127 > YourFont18pt7b.h
   ```
   - `18` = point size
   - `32 127` = ASCII character range (space to ~)

3. **Include in your project:**
   - Place the `.h` file in `include/` folder
   - Include it: `#include "YourFont18pt7b.h"`
   - Use it: `tft->setFont(&YourFont18pt7b);`

### Method 2: Online Font Converter

Use this web tool: https://rop.nl/truetype2gfx/

1. Upload your TrueType (.ttf) or OpenType (.otf) font
2. Select character range and size
3. Download the generated `.h` file
4. Place in your `include/` folder

## Example: Science Gothic Font

If you want Science Gothic:

1. **Download Science Gothic** (free from various font sites)
2. **Convert it:**
   ```bash
   fontconvert ScienceGothic.ttf 14 32 127 > ScienceGothic14pt7b.h
   ```
3. **Add to project:**
   - Copy `ScienceGothic14pt7b.h` to `include/`
   - In disp.cpp: `#include "ScienceGothic14pt7b.h"`
   - Use: `tft->setFont(&ScienceGothic14pt7b);`

## Font Size Recommendations for 160x128 Display

- **Large numbers (tram times)**: 18-24pt
- **Labels ("Olga", "Jack")**: 9-12pt
- **Temperature**: 12-18pt
- **Small text (wind, data age)**: 9pt

## PNG Images - Answer to Your Question

**Short answer: NO, Adafruit GFX does NOT support PNG directly.**

### What IS Supported:

1. **Monochrome Bitmaps (1-bit)**
   - Use `drawBitmap()` function
   - Must convert PNG → C array
   
2. **16-bit RGB565 Bitmaps** (for color displays like ST7735)
   - Use `drawRGBBitmap()` function
   - Must convert PNG → C array in RGB565 format

### How to Display Images:

#### Option 1: Convert PNG to C Array

Use online tool: http://javl.github.io/image2cpp/

1. Upload your PNG
2. Set canvas size (e.g., 16x16 pixels for small icon)
3. Select output: Arduino code, horizontal, 1-bit per pixel
4. Copy the generated C array

Example output:
```cpp
const unsigned char icon_battery [] PROGMEM = {
  0x00, 0x00, 0x7f, 0xfe, 0x40, 0x02, 0x5f, 0xfa,
  0x5f, 0xfa, 0x5f, 0xfa, 0x40, 0x02, 0x7f, 0xfe
};
```

#### Option 2: Using Icons in Your Display

```cpp
// In disp.cpp (at top level, outside functions):
const unsigned char icon_battery [] PROGMEM = {
  // ... generated array ...
};

// To display:
tft->drawBitmap(x, y, icon_battery, 16, 16, ST77XX_WHITE);
```

### Practical Example for TramReader

```cpp
// Small tram icon (16x16):
const unsigned char icon_tram [] PROGMEM = {
  0x01, 0x80, 0x03, 0xc0, 0x07, 0xe0, 0x0f, 0xf0,
  0x1f, 0xf8, 0x3f, 0xfc, 0x7f, 0xfe, 0xff, 0xff,
  0xff, 0xff, 0x7f, 0xfe, 0x3f, 0xfc, 0x1f, 0xf8,
  0x0f, 0xf0, 0x07, 0xe0, 0x03, 0xc0, 0x01, 0x80
};

// Display the icon next to "Tram 17":
tft->drawBitmap(2, 5, icon_tram, 16, 16, ST77XX_YELLOW);
tft->setCursor(20, 15);
tft->print("Tram 17");
```

## Recommended Approach for Your Display

For the cleanest look on a small 160x128 display:

1. **Use FreeSansBold fonts** for numbers (built-in, no conversion needed)
2. **Keep icons simple** - 16x16 or 24x24 pixels max
3. **Use monochrome bitmaps** - easier to work with
4. **Stick with text** for most things - fonts are more flexible than images

Would you like me to:
1. Add FreeSansBold fonts to your tram time display?
2. Create some simple icons (battery, moisture, wind, etc.)?
3. Show you how to convert a specific PNG you have?
