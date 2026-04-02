#include <OFC_Ui.h>

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

void OFC_Ui::draw_scan_card() {
    clear_screen();
    //draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());

    time_t now_sec = time(nullptr);
    struct tm now_tm = {};
    localtime_r(&now_sec, &now_tm);

    bool has_booking_today = false;
    bool is_current_booking = false;
    char time_range[32] = {0};

    if (_next_booking->has_booking && _next_booking->start_unix > 0 && _next_booking->end_unix > _next_booking->start_unix) {
        time_t start_t = (time_t)_next_booking->start_unix;
        time_t end_t   = (time_t)_next_booking->end_unix;
        struct tm start_tm = {};
        localtime_r(&start_t, &start_tm);

        // check if the booking is today
        if (now_tm.tm_year == start_tm.tm_year &&
            now_tm.tm_mon  == start_tm.tm_mon  &&
            now_tm.tm_mday == start_tm.tm_mday) {
            has_booking_today = true;
            format_booking_time_range(*(_next_booking), time_range, sizeof(time_range));
        }
        // fallback: if time looks unsynced (e.g. 1970) but we have a booking, show it anyway
        else if (now_sec < 1000000000) {
            has_booking_today = true;
            format_booking_time_range(*(_next_booking), time_range, sizeof(time_range));
        }

        if (now_sec >= _next_booking->start_unix && now_sec < _next_booking->end_unix) {
            is_current_booking = true;
        }
    }

    if (is_current_booking) {
        // Machine booked now, waiting to be unlocked
        draw_center_background(255, 165, 0);  // orange
        draw_button_left("Manual");

        char booked_line[48];
        const char* user = (_next_booking->user_name[0] ? _next_booking->user_name : "?");
        snprintf(booked_line, sizeof(booked_line), "BOOKED by %s", user);
        printTFTcentered(booked_line, _tft->color565(0, 0, 0), 2, 0, 23, 320, 30);

        if (has_booking_today && time_range[0]) {
            printTFTcentered(time_range, _tft->color565(0, 0, 0), 2, 0, 43, 320, 30);
        }
    } else {
        // Machine free
        draw_center_background(100, 255, 100);
        draw_button_left("Manual");

        if (has_booking_today && time_range[0]) {
            printTFTcentered("FREE | Next:", _tft->color565(0, 0, 0), 2, 0, 23, 320, 30);
            printTFTcentered(time_range, _tft->color565(0, 0, 0), 2, 0, 43, 320, 30);
        } else {
            // No booking today or no future booking
            printTFTcentered("FREE", _tft->color565(0, 0, 0), 3, 0, 40, 320, 40);
        }
    }

    printTFTcentered("Scan card", _tft->color565(0, 0, 0), 2, 0, 90, 160, 30);
    printTFTcentered("to unlock", _tft->color565(0, 0, 0), 2, 0, 110, 160, 30);
    printTFTcentered("Or book here", _tft->color565(0, 0, 0), 2, 160, 150, 160, 30);
    _qr->draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 200, 75);

    _menu = SCAN_CARD;
}
