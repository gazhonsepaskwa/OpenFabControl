#include "../../OFC_Ui.h"

void OFC_Ui::menu_handler_book_session(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
        book_session_minutes += 5;
        draw_book_session_values();
    } else if (ev == EVENT_BTN_LEFT_LONG) {
        book_session_minutes -= 5;
        if (book_session_minutes < BOOK_SESSION_MIN_MINUTES) book_session_minutes = BOOK_SESSION_MIN_MINUTES;
        draw_book_session_values();
    } else if (ev == EVENT_BTN_RIGHT) {
        draw_scan_card(qr, menu);
    } else if (ev == EVENT_BTN_RIGHT_LONG) {
        String resource_uuid = preferences.getString(UUID_KEY, "");
        char errbuf[64] = {0};
        if (create_session(last_scanned_access_key, resource_uuid.c_str(), book_session_minutes, &current_session, errbuf, sizeof(errbuf))) {
            clear_screen();
            draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
            draw_center_background(60, 100, 140);
            printTFTcentered("Starting session...", h.tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
            printTFTcentered("Please wait", h.tft.color565(220, 220, 220), 2, 0, 100, 320, 30);
            // Give some margin so that started_at (set slightly in the future)
            // is definitely in the past when we effectively start using the machine.
            delay(5000);
            if (start_session(last_scanned_access_key, resource_uuid.c_str(), &current_session, errbuf, sizeof(errbuf))) {
                last_tick_ms = millis();
                h.relay_on();
                draw_machine_usage(menu);
            } else {
                const char* err = errbuf[0] ? errbuf : "Start session failed";
                show_session_error(err);
                delay(5000);
                draw_scan_card(qr, menu);
            }
        } else {
            show_session_error(errbuf[0] ? errbuf : "Book failed");
            delay(3000);
            draw_scan_card(qr, menu);
        }
    }
}
