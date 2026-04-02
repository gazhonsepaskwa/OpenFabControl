#include <OFC_Ui.h>
#include "OFC_Hardware.h"
#include <Arduino.h>
#include <Preferences.h>

// global vars
extern char         g_last_scanned_access_key[32];
extern Preferences  g_preferences;
extern OFC_Hardware g_hardware;
extern OFC_Network  g_network;
extern Session      g_current_session;

// init load the scan_card screen
void OFC_Ui::menu_handler_scan_card(Event ev) {
    switch (ev) {
        case Event::EVENT_BTN_LEFT:
            draw_machine_info();
            break;

        case Event::EVENT_CARD:
            char errbuf[64] = {0};
            if (g_network.api && g_network.api->start_session(g_last_scanned_access_key, &g_current_session, errbuf, sizeof(errbuf))) {
                g_hardware.relay_on();
                draw_machine_usage();
            } else {
                const char* err = errbuf[0] ? errbuf : "Start session failed";
                bool no_session = (strstr(errbuf, "no session") != nullptr) || (strstr(errbuf, "No session") != nullptr);
                if (no_session) {
                    _book_session_minutes = book_session_min_minutes;
                    draw_book_session();
                } else {
                    show_error_screen(err);
                    delay(5000);
                    draw_scan_card();
                }
            }
            break;
    }
}
