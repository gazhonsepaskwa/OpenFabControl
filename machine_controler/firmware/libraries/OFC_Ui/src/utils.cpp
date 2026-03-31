#include "OFC_Ui.h"

#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#include <cstdio>
#include <cstring>

void OFC_Ui::printTFT(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size) {
    _tft->setTextColor(color);
    _tft->setTextSize(size);
    _tft->setCursor(x, y);
    _tft->print(text);
}

void OFC_Ui::printTFTBold(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size) {
    _tft->setTextColor(color);
    _tft->setTextSize(size);
    _tft->setCursor(x, y);
    _tft->print(text);
    _tft->setCursor(x, y + 1);
    _tft->print(text);
}

void OFC_Ui::printTFTcentered(
    const char* text,
    uint16_t color,
    uint8_t size,
    uint16_t rx, uint16_t ry,
    uint16_t rw, uint16_t rh
) {
    if (!text) text = "";
    uint16_t textWidth  = (uint16_t)(strlen(text) * 6U * size);
    uint16_t textHeight = (uint16_t)(8U * size);

    int16_t cx = (int16_t)(rx + (rw - textWidth) / 2);
    int16_t cy = (int16_t)(ry + (rh - textHeight) / 2);

    if (cx < (int16_t)rx) cx = (int16_t)rx;
    if (cy < (int16_t)ry) cy = (int16_t)ry;

    _tft->setTextColor(color);
    _tft->setTextSize(size);
    _tft->setCursor(cx, cy);
    _tft->print(text);
}

void OFC_Ui::draw_button_left(const char* msg) {
    _tft->fillRect(0, 200, 155, 50, ILI9341_WHITE);
    printTFTcentered(msg, _tft->color565(0, 0, 0), 2, 0, 200, 155, 50);
}

void OFC_Ui::draw_button_right(const char* msg) {
    _tft->fillRect(165, 200, 155, 50, ILI9341_WHITE);
    printTFTcentered(msg, _tft->color565(0, 0, 0), 2, 165, 200, 155, 50);
}

void OFC_Ui::draw_button_left(const char* msg, uint8_t r, uint8_t g, uint8_t b) {
    _tft->fillRect(0, 200, 155, 50, _tft->color565(r, g, b));
    printTFTcentered(msg, _tft->color565(0, 0, 0), 2, 0, 200, 155, 50);
}

void OFC_Ui::draw_button_right(const char* msg, uint8_t r, uint8_t g, uint8_t b) {
    _tft->fillRect(165, 200, 155, 50, _tft->color565(r, g, b));
    printTFTcentered(msg, _tft->color565(0, 0, 0), 2, 165, 200, 155, 50);
}

void OFC_Ui::draw_title(const char* msg) {
    printTFT(msg, 5, 3, _tft->color565(255, 255, 255), 2);
}

void OFC_Ui::draw_center_background(uint8_t r, uint8_t g, uint8_t b) {
    _tft->fillRect(0, 23, 320, 167, _tft->color565(r, g, b));
}

void OFC_Ui::format_hms(int total_sec, char* out, size_t out_size) {
    if (!out || out_size == 0) return;
    if (total_sec < 0) total_sec = 0;
    int h = total_sec / 3600;
    int m = (total_sec % 3600) / 60;
    int s = total_sec % 60;
    snprintf(out, out_size, "%02d:%02d:%02d", h, m, s);
    out[out_size - 1] = '\0';
}

