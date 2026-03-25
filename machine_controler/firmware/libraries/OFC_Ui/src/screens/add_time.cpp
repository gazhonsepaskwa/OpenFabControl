#include "OFC_Ui.h"


// TODO do something of this

// static int add_time_selected_minutes = 0;
// static int add_time_max_minutes = 0;
// static bool add_time_unlimited = false;


// TODO see if i expose this or make a wraper (probably a wraper) : see OFC_Ui.h
void OFC_Ui::draw_add_time_values() {
    uint16_t bg = _tft->color565(60, 120, 180);
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

void OFC_Ui::draw_add_time() {
    clear_screen();
    draw_title((char*)_machine_name.c_str());
    draw_center_background(60, 120, 180);

    printTFTcentered("Add time", _tft->color565(255, 255, 255), 2, 0, 40, 320, 30);
    draw_add_time_values();

    // Buttons: explain tap/long press behavior
    draw_button_left((char*)"+5m | -5m");
    draw_button_right((char*)"<- | OK");

    _menu = ADD_TIME;
}
