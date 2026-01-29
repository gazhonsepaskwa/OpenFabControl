#pragma once

// to silent the undefined
class Adafruit_ILI9341;

void printTFT(char* text, int16_t x, int16_t y, uint16_t color, uint8_t size);
void printTFTBold(char* text, int16_t x, int16_t y, uint16_t color, uint8_t size);
void printTFTcentered( const char* text, uint16_t color, uint8_t size, uint16_t rx, uint16_t ry, uint16_t rw, uint16_t rh);

void draw_button_left(char* msg);
void draw_button_right(char* msg);
void draw_button_left(char* msg, uint8_t r, uint8_t g, uint8_t b);
void draw_button_right(char* msg, uint8_t r, uint8_t g, uint8_t b);
void draw_title(char* msg);
void draw_center_background(uint8_t r, uint8_t g, uint8_t b);
void draw_center_background(uint8_t r, uint8_t g, uint8_t b);
void clear_screen(Adafruit_ILI9341& tft);
