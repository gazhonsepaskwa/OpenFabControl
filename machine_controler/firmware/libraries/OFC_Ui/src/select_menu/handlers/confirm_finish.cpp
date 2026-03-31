#include "../../OFC_Ui.h"

void OFC_Ui::menu_handler_confirm_finish(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
        draw_machine_usage(menu);
    } else if (ev == EVENT_BTN_RIGHT) {
        String resource_uuid = preferences.getString(UUID_KEY, "");
        if (stop_session(resource_uuid.c_str())) {
            clear_screen();
            printTFTcentered("exiting session...", _tft->color565(255, 255, 255), 2, 0, 70, 320, 30);
            delay(2000);
            h.relay_off();
            force_refresh_next_booking();
            draw_scan_card(qr, menu);
        } else {
            show_session_error("Stop session failed");
            delay(5000);
            draw_confirm_finish(menu);
        }
    }
}
