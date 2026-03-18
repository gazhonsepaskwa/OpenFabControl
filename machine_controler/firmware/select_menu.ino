#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <cstdint>
#include <ctime>
#include <time.h>
#include <QRCodeGFX.h>

#include "firmware.h"
#include "screen_utils.h"
#include <string.h>

// External reference to global h object defined in firmware.ino
extern OFC_Hardware h;

// Add time
static int add_time_selected_minutes = 0;
static int add_time_max_minutes = 0;
static bool add_time_unlimited = false;

// Book session (from machine when no session to start)
static int book_session_minutes = BOOK_SESSION_MIN_MINUTES;

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

void draw_machine_info(QRCodeGFX& qr, Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_button_left("<- Back");
    printTFTcentered( "User Manual",   h.tft.color565(0, 0, 0), 2, 0, 150, 160, 30);
    qr.draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 120, 75);
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
    h.tft.fillRect(0, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, h.tft.color565(255, 255, 255), 3, 0, 95, 160, 45);

    format_hms(time_used_val, buf, sizeof(buf));
    h.tft.fillRect(160, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, h.tft.color565(255, 255, 255), 3, 160, 95, 160, 45);
}

void update_machine_usage_times(void) {
    draw_machine_usage_times_inner();
}

void draw_machine_usage(Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_button_left("Add time");
    draw_button_right("Finish", 255, 100, 100);
    printTFTcentered("Time Left:", h.tft.color565(255, 255, 255), 2, 0, 70, 160, 30);
    printTFTcentered("Time used:", h.tft.color565(255, 255, 255), 2, 160, 70, 160, 30);
    draw_machine_usage_times_inner();
    menu = MACHINE_USAGE;
}

static void draw_add_time_values(void) {
    uint16_t bg = h.tft.color565(60, 120, 180);
    h.tft.fillRect(0, 80, 320, 40, bg);
    char buf[32];
    snprintf(buf, sizeof(buf), "+%d min", add_time_selected_minutes);
    printTFTcentered(buf, h.tft.color565(255, 255, 255), 3, 0, 80, 320, 40);

    h.tft.fillRect(0, 130, 320, 25, bg);
    if (add_time_unlimited) {
        printTFTcentered("Max: unlimited", h.tft.color565(230, 230, 230), 2, 0, 130, 320, 25);
    } else if (add_time_max_minutes == 0) {
        printTFTcentered("No extra time available", h.tft.color565(255, 200, 200), 2, 0, 130, 320, 25);
    } else if (add_time_max_minutes > 0) {
        char maxbuf[32];
        snprintf(maxbuf, sizeof(maxbuf), "Max: %d min", add_time_max_minutes);
        printTFTcentered(maxbuf, h.tft.color565(230, 230, 230), 2, 0, 130, 320, 25);
    }
}

static void draw_add_time_screen(Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_center_background(60, 120, 180);

    printTFTcentered("Add time", h.tft.color565(255, 255, 255), 2, 0, 40, 320, 30);
    draw_add_time_values();

    // Buttons: explain tap/long press behavior
    draw_button_left((char*)"+5m | -5m");
    draw_button_right((char*)"<- | OK");

    menu = ADD_TIME;
}
void draw_confirm_finish(Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_button_left("<- Back");
    draw_button_right("Confirm", 255, 100, 100);
    printTFTcentered("Finish session early ?", h.tft.color565(255, 100, 100), 2, 0, 23, 320, 167);
    menu = CONFIRM_FINISH;
}

static void draw_book_session_values(void) {
    uint16_t bg = h.tft.color565(80, 140, 80);
    h.tft.fillRect(0, 80, 320, 40, bg);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d min", book_session_minutes);
    printTFTcentered(buf, h.tft.color565(255, 255, 255), 3, 0, 80, 320, 40);
    h.tft.fillRect(0, 125, 320, 25, bg);
    printTFTcentered("Min 10 min", h.tft.color565(230, 230, 230), 2, 0, 125, 320, 25);
}

static void draw_book_session_screen(Menu& menu) {
    clear_screen();
    draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
    draw_center_background(80, 140, 80);

    printTFTcentered("Book session", h.tft.color565(255, 255, 255), 2, 0, 40, 320, 30);
    draw_book_session_values();

    draw_button_left((char*)"+5m | -5m");
    draw_button_right((char*)"X | OK");

    menu = BOOK_SESSION;
}
