#include "../OFC_Ui.h"

void OFC_Ui::update_menu(Event ev) {
    switch (_menu) {
        case INIT:
            menu_handler_init(ev);
            break;
        case SCAN_CARD:
            if (ev == EVENT_BTN_LEFT) {
                draw_machine_info(qr, menu);
            } else if (ev == EVENT_CARD) {
                Serial.print("Scanned Badge : ");
                Serial.println(last_scanned_access_key);
                String resource_uuid = preferences.getString(UUID_KEY, "");
                char errbuf[64] = {0};
                if (start_session(last_scanned_access_key, resource_uuid.c_str(), &current_session, errbuf, sizeof(errbuf))) {
                    last_tick_ms = millis();
                    h.relay_on();
                    draw_machine_usage(menu);
                } else {
                    const char* err = errbuf[0] ? errbuf : "Start session failed";
                    bool no_session = (strstr(errbuf, "no session") != nullptr) || (strstr(errbuf, "No session") != nullptr);
                    if (no_session) {
                        book_session_minutes = BOOK_SESSION_MIN_MINUTES;
                        draw_book_session_screen(menu);
                    } else {
                        show_session_error(err);
                        delay(5000);
                        draw_scan_card(qr, menu);
                    }
                }
            }
            break;
        case MACHINE_INFO:
            if (ev == EVENT_BTN_LEFT) { draw_scan_card(qr, menu); }
            break;
        case MACHINE_USAGE:
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
            break;
        case ADD_TIME:
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
            break;
        case BOOK_SESSION:
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
            break;
        case CONFIRM_FINISH:
            if (ev == EVENT_BTN_LEFT) {
                draw_machine_usage(menu);
            } else if (ev == EVENT_BTN_RIGHT) {
                String resource_uuid = preferences.getString(UUID_KEY, "");
                if (stop_session(resource_uuid.c_str())) {
                    clear_screen();
                    printTFTcentered("exiting session...", h.tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
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
            break;
    }
}

void OFC_Ui::update_menu(Event ev) {
    switch (_menu) {
        case INIT:
            menu_handler_init(ev);
            break;
        case SCAN_CARD:
            menu_handler_scan_card(ev);
            break;
        case MACHINE_INFO:
            if (ev == EVENT_BTN_LEFT) { draw_scan_card(qr, menu); }
            break;
        case MACHINE_USAGE:
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
            break;
        case ADD_TIME:
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
            break;
        case BOOK_SESSION:
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
            break;
        case CONFIRM_FINISH:
            if (ev == EVENT_BTN_LEFT) {
                draw_machine_usage(menu);
            } else if (ev == EVENT_BTN_RIGHT) {
                String resource_uuid = preferences.getString(UUID_KEY, "");
                if (stop_session(resource_uuid.c_str())) {
                    clear_screen();
                    printTFTcentered("exiting session...", h.tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
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
            break;
    }
}
