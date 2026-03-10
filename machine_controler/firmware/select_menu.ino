#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <cstdint>
#include <ctime>
#include <time.h>
#include <QRCodeGFX.h>

#include "firmware.h"
#include "screen_utils.h"
#include "utils.h"

extern Adafruit_ILI9341 tft;

// Add time
static int add_time_selected_minutes = 0;
static int add_time_max_minutes = 0;
static bool add_time_unlimited = false;

static void format_booking_time_range(const NextBooking& booking, char* out, size_t out_size) {
    if (!booking.has_booking || out_size == 0) {
        if (out_size) out[0] = '\0';
        return;
    }

    time_t start_t = (time_t)booking.start_unix;
    time_t end_t   = (time_t)booking.end_unix;
    struct tm start_tm = {};
    struct tm end_tm   = {};

    localtime_r(&start_t, &start_tm);
    localtime_r(&end_t, &end_tm);

    char start_buf[16];
    char end_buf[16];
    strftime(start_buf, sizeof(start_buf), "%Hh%M", &start_tm);
    strftime(end_buf,   sizeof(end_buf),   "%Hh%M", &end_tm);

    snprintf(out, out_size, "%s -> %s", start_buf, end_buf);
}

void draw_scan_card(QRCodeGFX& qr, Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());

    time_t now_sec = time(nullptr);
    struct tm now_tm = {};
    localtime_r(&now_sec, &now_tm);

    bool has_booking_today = false;
    bool is_current_booking = false;
    char time_range[32] = {0};

    if (next_booking.has_booking && next_booking.start_unix > 0 && next_booking.end_unix > next_booking.start_unix) {
        time_t start_t = (time_t)next_booking.start_unix;
        time_t end_t   = (time_t)next_booking.end_unix;
        struct tm start_tm = {};
        localtime_r(&start_t, &start_tm);

        // check if the booking is today
        if (now_tm.tm_year == start_tm.tm_year &&
            now_tm.tm_mon  == start_tm.tm_mon  &&
            now_tm.tm_mday == start_tm.tm_mday) {
            has_booking_today = true;
            format_booking_time_range(next_booking, time_range, sizeof(time_range));
        }
        // fallback: if time looks unsynced (e.g. 1970) but we have a booking, show it anyway
        else if (now_sec < 1000000000) {
            has_booking_today = true;
            format_booking_time_range(next_booking, time_range, sizeof(time_range));
        }

        if (now_sec >= next_booking.start_unix && now_sec < next_booking.end_unix) {
            is_current_booking = true;
        }
    }

    if (is_current_booking) {
        // Machine booked now, waiting to be unlocked
        draw_center_background(255, 165, 0);  // orange
        draw_button_left("Manual");

        char booked_line[48];
        const char* user = (next_booking.user_name[0] ? next_booking.user_name : "?");
        snprintf(booked_line, sizeof(booked_line), "BOOKED by %s", user);
        printTFTcentered(booked_line, tft.color565(0, 0, 0), 2, 0, 23, 320, 30);

        if (has_booking_today && time_range[0]) {
            printTFTcentered(time_range, tft.color565(0, 0, 0), 2, 0, 43, 320, 30);
        }
    } else {
        // Machine free
        draw_center_background(100, 255, 100);
        draw_button_left("Manual");

        if (has_booking_today && time_range[0]) {
            printTFTcentered("FREE | Next:", tft.color565(0, 0, 0), 2, 0, 23, 320, 30);
            printTFTcentered(time_range, tft.color565(0, 0, 0), 2, 0, 43, 320, 30);
        } else {
            // No booking today or no future booking
            printTFTcentered("FREE", tft.color565(0, 0, 0), 3, 0, 40, 320, 40);
        }
    }

    printTFTcentered("Scan card", tft.color565(0, 0, 0), 2, 0, 90, 160, 30);
    printTFTcentered("to unlock", tft.color565(0, 0, 0), 2, 0, 110, 160, 30);
    printTFTcentered("Or book here",   tft.color565(0, 0, 0), 2, 160, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 230, 100);

    menu = SCAN_CARD;
}

void draw_machine_info(QRCodeGFX& qr, Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_button_left("<- Back");
    printTFTcentered( "User Manual",   tft.color565(0, 0, 0), 2, 0, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 150, 100);
    menu = MACHINE_INFO;
}

void draw_machine_usage_times_inner(void) {
    time_t now_sec = time(nullptr);
    int time_left = (int)(current_session.ended_at_unix - now_sec);
    int time_used_val = (int)(now_sec - current_session.started_at_unix);
    if (time_left < 0) time_left = 0;
    if (time_used_val < 0) time_used_val = 0;

    char buf[16];
    format_hms(time_left, buf, sizeof(buf));
    tft.fillRect(0, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, tft.color565(255, 255, 255), 3, 0, 95, 160, 45);

    format_hms(time_used_val, buf, sizeof(buf));
    tft.fillRect(160, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, tft.color565(255, 255, 255), 3, 160, 95, 160, 45);
}

void update_machine_usage_times(void) {
    draw_machine_usage_times_inner();
}

void draw_machine_usage(Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_button_left("Add time");
    draw_button_right("Finish", 255, 100, 100);
    printTFTcentered("Time Left:", tft.color565(255, 255, 255), 2, 0, 70, 160, 30);
    printTFTcentered("Time used:", tft.color565(255, 255, 255), 2, 160, 70, 160, 30);
    draw_machine_usage_times_inner();
    menu = MACHINE_USAGE;
}

static void draw_add_time_values(void) {
    uint16_t bg = tft.color565(60, 120, 180);
    tft.fillRect(0, 80, 320, 40, bg);
    char buf[32];
    snprintf(buf, sizeof(buf), "+%d min", add_time_selected_minutes);
    printTFTcentered(buf, tft.color565(255, 255, 255), 3, 0, 80, 320, 40);

    tft.fillRect(0, 130, 320, 25, bg);
    if (add_time_unlimited) {
        printTFTcentered("Max: unlimited", tft.color565(230, 230, 230), 2, 0, 130, 320, 25);
    } else if (add_time_max_minutes == 0) {
        printTFTcentered("No extra time available", tft.color565(255, 200, 200), 2, 0, 130, 320, 25);
    } else if (add_time_max_minutes > 0) {
        char maxbuf[32];
        snprintf(maxbuf, sizeof(maxbuf), "Max: %d min", add_time_max_minutes);
        printTFTcentered(maxbuf, tft.color565(230, 230, 230), 2, 0, 130, 320, 25);
    }
}

static void draw_add_time_screen(Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_center_background(60, 120, 180);

    printTFTcentered("Add time", tft.color565(255, 255, 255), 2, 0, 40, 320, 30);
    draw_add_time_values();

    // Buttons: explain tap/long press behavior
    draw_button_left((char*)"T +5m | L -5m");
    draw_button_right((char*)"T <- | L OK");

    menu = ADD_TIME;
}
void draw_confirm_finish(Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_button_left("<- Back");
    draw_button_right("Confirm", 255, 100, 100);
    printTFTcentered("Finish session early ?", tft.color565(255, 100, 100), 2, 0, 23, 320, 167);
    menu = CONFIRM_FINISH;
}

void select_menu(QRCodeGFX& qr, Menu& menu, Event ev) {
    switch (menu) {
        case INIT:
            draw_scan_card(qr, menu);
            break;
        case SCAN_CARD:
            if (ev == EVENT_BTN_LEFT) {
                draw_machine_info(qr, menu);
            } else if (ev == EVENT_CARD) {
                Serial.print("Scanned Badge : ");
                Serial.println(last_scanned_access_key);
                String resource_uuid = preferences.getString(UUID_KEY, "");
                char errbuf[64];
                if (start_session(last_scanned_access_key, resource_uuid.c_str(), &current_session, errbuf, sizeof(errbuf))) {
                    last_tick_ms = millis();
                    relay_on();
                    draw_machine_usage(menu);
                } else {
                    show_session_error(errbuf[0] ? errbuf : "Start session failed");
                    delay(5000);
                    draw_scan_card(qr, menu);
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
        case CONFIRM_FINISH:
            if (ev == EVENT_BTN_LEFT) {
                draw_machine_usage(menu);
            } else if (ev == EVENT_BTN_RIGHT) {
                String resource_uuid = preferences.getString(UUID_KEY, "");
                if (stop_session(resource_uuid.c_str())) {
                    clear_screen();
                    printTFTcentered("exiting session...", tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
                    delay(2000);
                    relay_off();
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
