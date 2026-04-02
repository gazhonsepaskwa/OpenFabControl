#include <OFC_Ui.h>

extern OFC_Network g_network;
extern Session g_current_session;

void OFC_Ui::menu_handler_add_time(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
        _add_time_selected_minutes += 5;
        if (!_add_time_unlimited && _add_time_max_minutes >= 0 && _add_time_selected_minutes > _add_time_max_minutes) {
            _add_time_selected_minutes = _add_time_max_minutes;
        }
        draw_add_time_values();
    } else if (ev == EVENT_BTN_LEFT_LONG) {
        _add_time_selected_minutes -= 5;
        if (_add_time_selected_minutes < 0) _add_time_selected_minutes = 0;
        draw_add_time_values();
    } else if (ev == EVENT_BTN_RIGHT) {
        draw_machine_usage();
    } else if (ev == EVENT_BTN_RIGHT_LONG) {
        if (_add_time_selected_minutes <= 0) {
            show_error_screen("No time to add");
            delay(2000);
            draw_add_time();
        } else {
            char errbuf[64] = {0};
            if (g_network.api && g_network.api->add_time(_add_time_selected_minutes, &g_current_session, errbuf, sizeof(errbuf))) {
                draw_machine_usage();
            } else {
                show_error_screen(errbuf[0] ? errbuf : "Add time failed");
                delay(3000);
                draw_machine_usage();
            }
        }
    }
}
