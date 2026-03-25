#include "OFC_Ui.h"
#include "OFC_Network.h" // For next_booking
#include "QRCodeGFX.h"

// Default constructor (only for global declaration)
OFC_Ui::OFC_Ui()
    :   _machine_name(""),
        _tft(0x0),
        _qr(0x0)
{}

// Constructor
OFC_Ui::OFC_Ui(String machine_name, Adafruit_ILI9341* tft, NextBooking* next_booking)
    :   _machine_name(machine_name),
        _tft(tft),
        _qr(new QRCodeGFX(*tft)),
        _menu(INIT),
        _next_booking(next_booking){
    _qr->setScale(2);  // 1 = default size, 2 = double, etc.
}

OFC_Ui::~OFC_Ui() {
    if (_qr) { delete _qr; }
}

void OFC_Ui::clear_screen() {
    _tft->fillScreen(ILI9341_BLACK);
}

void OFC_Ui::waiting_approval() {
    // generated with lopaka.com
    _tft->setTextColor(0xEBC7);
    _tft->setTextSize(2);
    _tft->setTextWrap(false);
    _tft->setCursor(24, 51);
    _tft->print("Waiting for approval...");
    _tft->drawRect(24, 120, 273, 59, 0xB653);
    _tft->setTextColor(0xFFFF);
    _tft->setTextSize(1);
    _tft->setCursor(89, 138);
    _tft->print("Please go to admin panel");
    _tft->setCursor(92, 154);
    _tft->print("and approve the machine");
}

Menu OFC_Ui::get_menu() {
    return _menu;
}
