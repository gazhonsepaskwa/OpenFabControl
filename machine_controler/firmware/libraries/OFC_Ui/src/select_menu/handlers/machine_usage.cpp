#include "../../OFC_Ui.h"

void OFC_Ui::menu_handler_machine_usage(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
        String resource_uuid = preferences.getString(UUID_KEY, "");
        char errbuf[64] = {0};
        int max_add = 0;
        if (!get_max_add_time(resource_uuid.c_str(), &max_add, errbuf, sizeof(errbuf))) {
            show_session_error(errbuf[0] ? errbuf : "Get max add time failed");
            delay(3000);
            draw_machine_usage(menu);
        } else {
            add_time_selected_minutes = 0;
            add_time_max_minutes = max_add;
            add_time_unlimited = (max_add == -1);
            draw_add_time_screen(menu);
        }
    }
    else if (ev == EVENT_BTN_RIGHT) { draw_confirm_finish(menu); }
}
