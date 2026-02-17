#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <cstdint>
#include <QRCodeGFX.h>

#include "firmware.h"
#include "screen_utils.h"

extern Adafruit_ILI9341 tft;

void draw_scan_card_no_reservation(QRCodeGFX& qr, Menu& menu) {
    clear_screen();
    draw_title("Zone Metal : TIG");
    draw_center_background(100, 255, 100);
    draw_button_left("Manual");

    printTFTcentered( "FREE | Next:", tft.color565(0, 0, 0), 2, 0, 23, 320, 30);
    printTFTcentered( "15h30 -> 17h00", tft.color565(0, 0, 0), 2, 0, 43, 320, 30);
    printTFTcentered( "Scan card", tft.color565(0, 0, 0), 2, 0, 90, 160, 30);
    printTFTcentered( "to unlock", tft.color565(0, 0, 0), 2, 0, 110, 160, 30);
    printTFTcentered( "Or book here",   tft.color565(0, 0, 0), 2, 160, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 230, 100);

    menu = SCAN_CARD_NO_RESERVATION;
}
void draw_machine_info(QRCodeGFX& qr, Menu& menu) {
    clear_screen();
    draw_button_left("<- Back");
    printTFT("Zone Metal : TIG", 5, 3, tft.color565(255, 255, 255), 2);
    printTFTcentered( "User Manual",   tft.color565(0, 0, 0), 2, 0, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 150, 100);
    menu = MACHINE_INFO;
}
void draw_machine_usage(Menu& menu) {
    clear_screen();
    draw_title("Zone Metal : TIG");
    draw_button_left("Add time");
    draw_button_right("Finish", 255, 100, 100);
    printTFTcentered( "Time Left:", tft.color565(255, 255, 255), 2, 0, 70, 160, 30);
    printTFTcentered( "00:45:00",   tft.color565(255, 255, 255), 3, 0, 100, 160, 30);
    printTFTcentered( "Time used:", tft.color565(255, 255, 255), 2, 160, 70, 160, 30);
    printTFTcentered( "00:00:00",   tft.color565(255, 255, 255), 3, 160, 100, 160, 30);
    menu = MACHINE_USAGE;
}
void draw_confirm_finish(Menu& menu) {
    clear_screen();
    draw_title("Zone Metal : TIG");
    draw_button_left("<- Back");
    draw_button_right("Confirm", 255, 100, 100);
    printTFTcentered("Finish session early ?", tft.color565(255, 100, 100), 2, 0, 23, 320, 167);
    menu = CONFIRM_FINISH;
}

void select_menu(QRCodeGFX& qr, Menu& menu, Event ev) {
    switch (menu) {
        case INIT:
            draw_scan_card_no_reservation(qr, menu);
            break;
        case SCAN_CARD_NO_RESERVATION:
            if      (ev == EVENT_BTN_LEFT)  { draw_machine_info(qr, menu); }
            else if (ev == EVENT_CARD)      { draw_machine_usage(menu); }
            break;
        case MACHINE_INFO:
            if      (ev == EVENT_BTN_LEFT)  { draw_scan_card_no_reservation(qr, menu); }
            break;
        case MACHINE_USAGE:
            if      (ev == EVENT_BTN_LEFT)  {  } // to-do
            else if (ev == EVENT_BTN_RIGHT) { draw_confirm_finish(menu); }
            break;
        case CONFIRM_FINISH:
            if      (ev == EVENT_BTN_LEFT)  { draw_machine_usage(menu); }
            else if (ev == EVENT_BTN_RIGHT) { draw_scan_card_no_reservation(qr, menu); }
    }
}
