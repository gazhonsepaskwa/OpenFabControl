#include <OFC_Ui.h>

extern OFC_Network g_network;

void OFC_Ui::menu_handler_machine_usage(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
        char errbuf[64] = {0};
        int max_add = 0;
        if (!(g_network.api && g_network.api->get_max_add_time(&max_add, errbuf, sizeof(errbuf)))) {
            show_error_screen(errbuf[0] ? errbuf : "Get max add time failed");
            delay(3000);
            draw_machine_usage();
        } else {
            _add_time_selected_minutes = 0;
            _add_time_max_minutes = max_add;
            _add_time_unlimited = (max_add == -1);
            draw_add_time();
        }
    }
    else if (ev == EVENT_BTN_RIGHT) { draw_confirm_finish(); }
}
