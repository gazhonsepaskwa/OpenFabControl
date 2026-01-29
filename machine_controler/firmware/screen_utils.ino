#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen

// External reference to global tft object defined in firmware.ino
extern Adafruit_ILI9341 tft;

// TFT utils
void printTFT(char* text, int16_t x, int16_t y, uint16_t color, uint8_t size) {
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
}
void printTFTBold(char* text, int16_t x, int16_t y, uint16_t color, uint8_t size) {
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
    tft.setCursor(x, y+1);
    tft.print(text);
}
void printTFTcentered(
    const char* text,
    uint16_t color,
    uint8_t size,
    uint16_t rx, uint16_t ry,
    uint16_t rw, uint16_t rh
) {
    // Calculate text dimensions
    uint16_t textWidth  = strlen(text) * 6 * size;
    uint16_t textHeight = 8 * size;

    // Calculate centered position
    int16_t cx = rx + (rw - textWidth) / 2;
    int16_t cy = ry + (rh - textHeight) / 2;

    // Clamp to rectangle (safety)
    if (cx < rx) cx = rx;
    if (cy < ry) cy = ry;

    // Draw text
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(cx, cy);
    tft.print(text);
}

// drawing utils
void draw_button_left(char* msg) {
    tft.fillRect(0, 200, 155,  50, ILI9341_WHITE);
    printTFTcentered(msg, tft.color565(0, 0, 0), 2, 0, 200, 155, 50);
}
void draw_button_right(char* msg) {
    tft.fillRect(165, 200, 155,  50, ILI9341_WHITE);
    printTFTcentered(msg, tft.color565(0, 0, 0), 2, 165, 200, 155, 50);
}
void draw_button_left(char* msg, uint8_t r, uint8_t g, uint8_t b) {
    tft.fillRect(0, 200, 155,  50, tft.color565(r, g, b));
    printTFTcentered(msg, tft.color565(0, 0, 0), 2, 0, 200, 155, 50);
}
void draw_button_right(char* msg, uint8_t r, uint8_t g, uint8_t b) {
    tft.fillRect(165, 200, 155,  50, tft.color565(r, g, b));
    printTFTcentered(msg, tft.color565(0, 0, 0), 2, 165, 200, 155, 50);
}
void draw_title(char* msg) {
    printTFT(msg, 5, 3, tft.color565(255, 255, 255), 2);
}
void draw_center_background(uint8_t r, uint8_t g, uint8_t b) {
    tft.fillRect(0,  23, 320, 167, tft.color565(r, g, b));
}
void clear_screen(Adafruit_ILI9341& tft) {
    tft.fillScreen(ILI9341_BLACK);
}
