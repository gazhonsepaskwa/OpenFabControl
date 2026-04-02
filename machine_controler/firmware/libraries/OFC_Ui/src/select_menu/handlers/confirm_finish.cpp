#include <OFC_Ui.h>
#include <OFC_Hardware.h>

extern OFC_Network g_network;
extern OFC_Hardware g_hardware;

void OFC_Ui::menu_handler_confirm_finish(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
        draw_machine_usage();
    } else if (ev == EVENT_BTN_RIGHT) {
        char errbuf[64] = {0};
        if (g_network.api && g_network.api->stop_session(errbuf, sizeof(errbuf))) {
            clear_screen();
            printTFTcentered("exiting session...", _tft->color565(255, 255, 255), 2, 0, 70, 320, 30);
            delay(2000);
            g_hardware.relay_off();
            if (g_network.api) g_network.api->force_refresh_next_booking();
            draw_scan_card();
        } else {
            show_error_screen(errbuf[0] ? errbuf : "Stop session failed");
            delay(5000);
            draw_confirm_finish();
        }
    }
}
