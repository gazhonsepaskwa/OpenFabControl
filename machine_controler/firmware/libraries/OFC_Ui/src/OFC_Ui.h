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
        void begin(String machine_name, Adafruit_ILI9341* tft, NextBooking* next_booking);

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
        int                 _book_session_minutes = 10;
        int                 _add_time_selected_minutes = 0;
        int                 _add_time_max_minutes = 0;
        bool                _add_time_unlimited = false;

        static constexpr int book_session_min_minutes = 10;

    // Public Methodes
    public:
        void clear_screen();
        void waiting_approval();
        void show_error_screen(const char* msg);
        void show_setup_error_and_restart(const char* msg);
        void show_setup_ap_instructions(const char* ap_ssid, const char* ap_pass);
        void show_setup_connecting_wifi(const char* ssid);
        void show_setup_registering();
        void show_setup_complete();
        void update_menu(Event ev);
        void update_machine_usage_times();
        Menu get_menu();


    // Temporary utils: screen helpers until lopaka.app is fully used
    private:
        void printTFT(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size);
        void printTFTBold(const char* text, int16_t x, int16_t y, uint16_t color, uint8_t size);
        void printTFTcentered(const char* text, uint16_t color, uint8_t size, uint16_t rx, uint16_t ry, uint16_t rw, uint16_t rh);

        void draw_button_left(const char* msg);
        void draw_button_right(const char* msg);
        void draw_button_left(const char* msg, uint8_t r, uint8_t g, uint8_t b);
        void draw_button_right(const char* msg, uint8_t r, uint8_t g, uint8_t b);
        void draw_title(const char* msg);
        void draw_center_background(uint8_t r, uint8_t g, uint8_t b);
        void format_hms(int total_sec, char* out, size_t out_size);
    
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

        void draw_book_session_values();
        void draw_add_time_values();
};

// If you see this, thanks for reading my code :D
// I've been putting so much time and efforts in this project (5 months at almost full time when i write this),
// I realy hope that some fablabs will use it.
