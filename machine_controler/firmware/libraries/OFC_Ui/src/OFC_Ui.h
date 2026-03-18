#pragma once
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <QRCodeGFX.h>

// list of the screens that the menu can be on
enum Menu {
    INIT,
    SCAN_CARD,
    MACHINE_INFO,
    MACHINE_USAGE,
    CONFIRM_FINISH,
    ADD_TIME,
    BOOK_SESSION
};

// list of events that the menu can receive
enum Event {
    EVENT_BTN_LEFT,
    EVENT_BTN_RIGHT,
    EVENT_CARD,
    EVENT_BTN_LEFT_LONG,
    EVENT_BTN_RIGHT_LONG,
    EVENT_NONE
};

// Class that manage displaying things to the screen
class OFC_Ui {
    // Constructor destructor
    public:
        OFC_Ui();
        OFC_Ui(String machine_name, Adafruit_ILI9341* tft);
       ~OFC_Ui();

    // Attribut
    private:
        String              _machine_name;
        Adafruit_ILI9341*   _tft;
        // menu selection
        QRCodeGFX*          _qr;
        Menu                _menu;

    // Methodes
    public:
        void clear_screen();
        void waiting_approval();
        void update_menu(Event ev);

    // Private Methodes ( Menu Handlers [ /handlers ] )
    private:
        void menu_handler_init(Event ev);
        void menu_handler_scan_card(Event ev);

    // Private Methodes ( Screens [ /screens ] )
    private:
        void draw_scan_card();
};

// If you see this, thanks for reading my code :D
// I've been putting so much time and efforts in this project (5 months at almost full time when i write this),
// I realy hope that some fablabs will use it.
