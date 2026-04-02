#include <OFC_Ui.h>
#include <OFC_Hardware.h>

extern OFC_Network g_network;
extern OFC_Hardware g_hardware;
extern Session g_current_session;
extern char g_last_scanned_access_key[32];

void OFC_Ui::menu_handler_book_session(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
        _book_session_minutes += 5;
        draw_book_session_values();
    } else if (ev == EVENT_BTN_LEFT_LONG) {
        _book_session_minutes -= 5;
        if (_book_session_minutes < book_session_min_minutes) _book_session_minutes = book_session_min_minutes;
        draw_book_session_values();
    } else if (ev == EVENT_BTN_RIGHT) {
        draw_scan_card();
    } else if (ev == EVENT_BTN_RIGHT_LONG) {
        char errbuf[64] = {0};
        if (g_network.api && g_network.api->create_session(g_last_scanned_access_key, _book_session_minutes, &g_current_session, errbuf, sizeof(errbuf))) {
            clear_screen();
            draw_title((char*)_machine_name.c_str());
            draw_center_background(60, 100, 140);
            printTFTcentered("Starting session...", _tft->color565(255, 255, 255), 2, 0, 70, 320, 30);
            printTFTcentered("Please wait", _tft->color565(220, 220, 220), 2, 0, 100, 320, 30);
            // Give some margin so that started_at (set slightly in the future)
            // is definitely in the past when we effectively start using the machine.
            delay(5000);
            if (g_network.api && g_network.api->start_session(g_last_scanned_access_key, &g_current_session, errbuf, sizeof(errbuf))) {
                g_hardware.relay_on();
                reset_session_active_usage_tracking();
                draw_machine_usage();
            } else {
                const char* err = errbuf[0] ? errbuf : "Start session failed";
                show_error_screen(err);
                delay(5000);
                draw_scan_card();
            }
        } else {
            show_error_screen(errbuf[0] ? errbuf : "Book failed");
            delay(3000);
            draw_scan_card();
        }
    }
}
