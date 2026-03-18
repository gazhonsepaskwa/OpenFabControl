#include "../../OFC_Ui.h"
#include "OFC_Hardware.h"
#include <Arduino.h>
#include <Preferences.h>

// global vars
extern char         g_last_scanned_access_key[32];
extern Preferences  g_preferences;
extern OFC_Hardware g_hardware;

// init load the scan_card screen
void OFC_Ui::menu_handler_scan_card(Event ev) {
    switch (ev) {
        case Event::EVENT_BTN_LEFT:
            //draw_machine_info(qr, menu);
            break;

        case Event::EVENT_CARD:
            String resource_uuid = g_preferences.getString(UUID_KEY, "");
            char errbuf[64] = {0};
            if (start_session(g_last_scanned_access_key, resource_uuid.c_str(), &current_session, errbuf, sizeof(errbuf))) {
                last_tick_ms = millis();
                g_hardware.relay_on();
                //draw_machine_usage(_menu);
            } else {
                const char* err = errbuf[0] ? errbuf : "Start session failed";
                bool no_session = (strstr(errbuf, "no session") != nullptr) || (strstr(errbuf, "No session") != nullptr);
                if (no_session) {
                    book_session_minutes = BOOK_SESSION_MIN_MINUTES;
                    //draw_book_session_screen(_menu);
                } else {
                    // show_session_error(err);
                    delay(5000);
                    draw_scan_card();
                }
            }
            break;
    }
}
