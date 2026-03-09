#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
#include <cstdio>
#include <cstring>

#include "firmware.h"

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
#define SCREEN_WIDTH      320

// Top-right status zone (same place as "SA" and refresh icon)
#define RIGHT_STATUS_X    (SCREEN_WIDTH - 32)
#define RIGHT_STATUS_Y    0
#define RIGHT_STATUS_W    32
#define RIGHT_STATUS_H    20

// Draw only the top-right status zone (no full screen update): refresh icon or "SA" or blank
void draw_title_right_status(bool refreshing) {
    tft.fillRect(RIGHT_STATUS_X, RIGHT_STATUS_Y, RIGHT_STATUS_W, RIGHT_STATUS_H, ILI9341_BLACK);
    if (refreshing) {
        uint16_t color = tft.color565(100, 180, 255);
        int cx = RIGHT_STATUS_X + RIGHT_STATUS_W / 2;
        int cy = RIGHT_STATUS_Y + RIGHT_STATUS_H / 2;
        tft.drawCircle(cx, cy, 6, color);
        // Small arrow head (circular refresh hint) at top-right of circle
        tft.drawLine(cx + 4, cy - 4, cx + 6, cy - 1, color);
        tft.drawLine(cx + 6, cy - 1, cx + 3, cy + 2, color);
    } else if (wifi_connection_lost) {
        tft.setTextColor(tft.color565(255, 200, 0));
        tft.setTextSize(2);
        tft.setCursor(RIGHT_STATUS_X + 4, 3);
        tft.print("SA");
    }
}

void draw_title(char* msg) {
    printTFT(msg, 5, 3, tft.color565(255, 255, 255), 2);
    draw_title_right_status(false);
}
void draw_center_background(uint8_t r, uint8_t g, uint8_t b) {
    tft.fillRect(0,  23, 320, 167, tft.color565(r, g, b));
}
void clear_screen(void) {
    tft.fillScreen(ILI9341_BLACK);
}

void format_hms(int total_sec, char* out, size_t out_size) {
    if (total_sec < 0) total_sec = 0;
    int h = total_sec / 3600;
    int m = (total_sec % 3600) / 60;
    int s = total_sec % 60;
    snprintf(out, out_size, "%02d:%02d:%02d", h, m, s);
}
