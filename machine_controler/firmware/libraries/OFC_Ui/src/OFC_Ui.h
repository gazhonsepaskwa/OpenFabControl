#pragma once
#include "OFC_Network.h"
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
        OFC_Ui(String machine_name, Adafruit_ILI9341* tft, NextBooking* next_booking);
       ~OFC_Ui();

    // Attribut
    private:
        String              _machine_name;
        Adafruit_ILI9341*   _tft; // poiter to OFC_Hardware::_tft
        // ~ menu ~
        Menu                _menu;
        QRCodeGFX*          _qr;
        NextBooking*        _next_booking; // pointer to OFC_Hardware::_next_booking
        // timers ( millis() val )
        unsigned long       _last_machine_usage_time_update_ms;

    // Public Methodes
    public:
        void clear_screen();
        void waiting_approval();
        void update_menu(Event ev);
        void update_machine_usage_times();
        Menu get_menu();

    // Private Methodes ( Menu Handlers [ /handlers ] )
    private:
        void menu_handler_init(Event ev);
        void menu_handler_scan_card(Event ev);
        void menu_handler_machine_info(Event ev);
        void menu_handler_machine_usage(Event ev);
        void menu_handler_add_time(Event ev);
        void menu_handler_book_session(Event ev);
        void menu_handler_confirm_finish(Event ev);

    // Private Methodes ( Screens [ /screens ] )
    private:
        void draw_scan_card();
        void draw_machine_usage();
        void draw_machine_usage_times_inner();
        void draw_machine_info();
        void draw_add_time();
        void draw_confirm_finish();
        void draw_book_session();

        // TODO see if i expose those or make a wraper (probably a wraper)
            // void draw_book_session_values();
            // void draw_add_time_values();
};

// If you see this, thanks for reading my code :D
// I've been putting so much time and efforts in this project (5 months at almost full time when i write this),
// I realy hope that some fablabs will use it.
