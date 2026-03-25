#include "../OFC_Ui.h"


void OFC_Ui::update_machine_usage_times() {
    // check last update
    if ((millis() - _last_machine_usage_time_update_ms) >= 1000) {
        // redraw numbers in if it's been one second
        draw_machine_usage_times_inner();
        _last_machine_usage_time_update_ms = millis();
    }
}

void OFC_Ui::draw_machine_usage_times_inner() {
    time_t now_sec = time(nullptr);
    int time_left = (int)(current_session.ended_at_unix - now_sec);
    int time_used_val = (int)(now_sec - current_session.started_at_unix);
    if (time_left < 0) time_left = 0;
    if (time_used_val < 0) time_used_val = 0;

    char buf[16];
    format_hms(time_left, buf, sizeof(buf));
    _tft->fillRect(0, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, _tft->color565(255, 255, 255), 3, 0, 95, 160, 45);

    format_hms(time_used_val, buf, sizeof(buf));
    _tft->fillRect(160, 95, 160, 45, ILI9341_BLACK);
    printTFTcentered(buf, _tft->color565(255, 255, 255), 3, 160, 95, 160, 45);
}

void OFC_Ui::draw_machine_usage() {
    clear_screen();
    draw_title((char*)_machine_name.c_str());
    draw_button_left("Add time");
    draw_button_right("Finish", 255, 100, 100);
    printTFTcentered("Time Left:", _tft->color565(255, 255, 255), 2, 0, 70, 160, 30);
    printTFTcentered("Time used:", _tft->color565(255, 255, 255), 2, 160, 70, 160, 30);
    draw_machine_usage_times_inner();
    _menu = MACHINE_USAGE;
}
