#include "../../OFC_Ui.h"

void OFC_Ui::menu_handler_add_time(Event ev) {
    if (ev == EVENT_BTN_LEFT) {
                    add_time_selected_minutes += 5;
                    if (!add_time_unlimited && add_time_max_minutes >= 0 && add_time_selected_minutes > add_time_max_minutes) {
                        add_time_selected_minutes = add_time_max_minutes;
                    }
                    draw_add_time_values();
                } else if (ev == EVENT_BTN_LEFT_LONG) {
                    add_time_selected_minutes -= 5;
                    if (add_time_selected_minutes < 0) add_time_selected_minutes = 0;
                    draw_add_time_values();
                } else if (ev == EVENT_BTN_RIGHT) {
                    // Cancel and go back
                    draw_machine_usage(menu);
                } else if (ev == EVENT_BTN_RIGHT_LONG) {
                    if (add_time_selected_minutes <= 0) {
                        show_session_error("No time to add");
                        delay(2000);
                        draw_add_time_screen(menu);
                    } else {
                        String resource_uuid = preferences.getString(UUID_KEY, "");
                        char errbuf[64] = {0};
                        if (add_time(resource_uuid.c_str(), add_time_selected_minutes, &current_session, errbuf, sizeof(errbuf))) {
                            last_tick_ms = millis();
                            draw_machine_usage(menu);
                        } else {
                            show_session_error(errbuf[0] ? errbuf : "Add time failed");
                            delay(3000);
                            draw_machine_usage(menu);
                        }
                    }
                }
}
