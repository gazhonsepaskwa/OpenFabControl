#pragma once
#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
#include <QRCodeGFX.h>

enum Menu {
    INIT,
    SCAN_CARD_NO_RESERVATION,
    MACHINE_INFO,
    MACHINE_USAGE,
    CONFIRM_FINISH
};

enum Event {
    EVENT_BTN_LEFT,
    EVENT_BTN_RIGHT,
    EVENT_CARD,
    EVENT_ANY
};

void select_menu(Adafruit_ILI9341& tft, QRCodeGFX& qr, Menu& menu, Event button);
