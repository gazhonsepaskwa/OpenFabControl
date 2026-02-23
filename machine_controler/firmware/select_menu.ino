#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <cstdint>
#include <ctime>
#include <QRCodeGFX.h>

#include "firmware.h"
#include "screen_utils.h"

extern Adafruit_ILI9341 tft;

void draw_scan_card_no_reservation(QRCodeGFX& qr, Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_center_background(100, 255, 100);
    draw_button_left("Manual");

    printTFTcentered( "FREE | Next:", tft.color565(0, 0, 0), 2, 0, 23, 320, 30);
    printTFTcentered( "15h30 -> 17h00", tft.color565(0, 0, 0), 2, 0, 43, 320, 30);
    printTFTcentered( "Scan card", tft.color565(0, 0, 0), 2, 0, 90, 160, 30);
    printTFTcentered( "to unlock", tft.color565(0, 0, 0), 2, 0, 110, 160, 30);
    printTFTcentered( "Or book here",   tft.color565(0, 0, 0), 2, 160, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 230, 100);

    menu = SCAN_CARD_NO_RESERVATION;
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
            draw_scan_card_no_reservation(qr, menu);
            break;
        case SCAN_CARD_NO_RESERVATION:
            if (ev == EVENT_BTN_LEFT) {
                draw_machine_info(qr, menu);
            } else if (ev == EVENT_CARD) {
                Serial.print("Scanned Badge : ");
                Serial.println(last_scanned_access_key);
                String resource_uuid = preferences.getString(UUID_KEY, "");
                char errbuf[64];
                if (start_session(last_scanned_access_key, resource_uuid.c_str(), &current_session, errbuf, sizeof(errbuf))) {
                    last_tick_ms = millis();
                    draw_machine_usage(menu);
                } else {
                    show_session_error(errbuf[0] ? errbuf : "Start session failed");
                    delay(5000);
                    draw_scan_card_no_reservation(qr, menu);
                }
            }
            break;
        case MACHINE_INFO:
            if (ev == EVENT_BTN_LEFT) { draw_scan_card_no_reservation(qr, menu); }
            break;
        case MACHINE_USAGE:
            if (ev == EVENT_BTN_LEFT) { }  // to-do Add time
            else if (ev == EVENT_BTN_RIGHT) { draw_confirm_finish(menu); }
            break;
        case CONFIRM_FINISH:
            if (ev == EVENT_BTN_LEFT) {
                draw_machine_usage(menu);
            } else if (ev == EVENT_BTN_RIGHT) {
                String resource_uuid = preferences.getString(UUID_KEY, "");
                if (stop_session(resource_uuid.c_str())) {
                    draw_scan_card_no_reservation(qr, menu);
                } else {
                    show_session_error("Stop session failed");
                    delay(5000);
                    draw_confirm_finish(menu);
                }
            }
            break;
    }
}
