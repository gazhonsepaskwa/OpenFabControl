#include <Arduino.h>
#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
#include <cstdint>
#include <QRCodeGFX.h>

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

void draw_scan_card_no_reservation(Adafruit_ILI9341& tft, QRCodeGFX& qr, Menu& menu) {
    // generic
    clear_screen(tft);
    draw_title("Zone Metal : TIG");
    draw_center_background(100, 255, 100);
    draw_button_left("Manual");

    // FREE + next usage
    printTFTcentered( "FREE | Next:", tft.color565(0, 0, 0), 2, 0, 23, 320, 30);
    printTFTcentered( "15h30 -> 17h00", tft.color565(0, 0, 0), 2, 0, 43, 320, 30);

    // scan card
    printTFTcentered( "Scan card", tft.color565(0, 0, 0), 2, 0, 90, 160, 30);
    printTFTcentered( "to unlock", tft.color565(0, 0, 0), 2, 0, 110, 160, 30);
    printTFTcentered( "Or book here",   tft.color565(0, 0, 0), 2, 160, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 230, 100);

    menu = SCAN_CARD_NO_RESERVATION;
}
void draw_machine_info(Adafruit_ILI9341& tft, QRCodeGFX& qr, Menu& menu) {
    clear_screen(tft);
    draw_button_left("<- Back");

    // machine name
    printTFT("Zone Metal : TIG", 5, 3, tft.color565(255, 255, 255), 2);

    // scan card
    printTFTcentered( "User Manual",   tft.color565(0, 0, 0), 2, 0, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 150, 100);
    menu = MACHINE_INFO;
}
void draw_machine_usage(Adafruit_ILI9341& tft, Menu& menu) {
    // generic
    clear_screen(tft);
    draw_title("Zone Metal : TIG");
    draw_button_left("Add time");
    draw_button_right("Finish", 255, 100, 100);

    // time left
    printTFTcentered( "Time Left:", tft.color565(255, 255, 255), 2, 0, 70, 160, 30);
    printTFTcentered( "00:45:00",   tft.color565(255, 255, 255), 3, 0, 100, 160, 30);

    // time used
    printTFTcentered( "Time used:", tft.color565(255, 255, 255), 2, 160, 70, 160, 30);
    printTFTcentered( "00:00:00",   tft.color565(255, 255, 255), 3, 160, 100, 160, 30);

    menu = MACHINE_USAGE;
}
void draw_confirm_finish(Adafruit_ILI9341& tft, Menu& menu) {
    clear_screen(tft);
    draw_title("Zone Metal : TIG");
    draw_button_left("<- Back");
    draw_button_right("Confirm", 255, 100, 100);

    // confirm text
    printTFTcentered("Finish session early ?", tft.color565(255, 100, 100), 2, 0, 23, 320, 167);
    menu = CONFIRM_FINISH;
}

void select_menu(Adafruit_ILI9341& tft, QRCodeGFX& qr, Menu &menu, Event ev) {
    switch (menu) {
        case INIT:
            draw_scan_card_no_reservation(tft, qr, menu);
            break;
        case SCAN_CARD_NO_RESERVATION:
            if      (ev == EVENT_BTN_LEFT)  { draw_machine_info(tft, qr, menu); }
            else if (ev == EVENT_CARD)      { draw_machine_usage(tft, menu); }
            break;
        case MACHINE_INFO:
            if      (ev == EVENT_BTN_LEFT)  { draw_scan_card_no_reservation(tft, qr, menu); }
            break;
        case MACHINE_USAGE:
            if      (ev == EVENT_BTN_LEFT)  {  } // to-do
            else if (ev == EVENT_BTN_RIGHT) { draw_confirm_finish(tft, menu); }
            break;
        case CONFIRM_FINISH:
            if      (ev == EVENT_BTN_LEFT)  { draw_machine_usage(tft, menu); }
            else if (ev == EVENT_BTN_RIGHT) { draw_scan_card_no_reservation(tft, qr, menu); }
    }
}
