#include "OFC_Ui.h"

// TODO do something of this

//static int book_session_minutes = BOOK_SESSION_MIN_MINUTES;

// TODO see if i expose this or make a wraper (probably a wraper) : see OFC_Ui.h
void OFC_Ui::draw_book_session_values() {
    uint16_t bg = _tft->color565(80, 140, 80);
    _tft->fillRect(0, 80, 320, 40, bg);
    char buf[32];
    snprintf(buf, sizeof(buf), "%d min", _book_session_minutes);
    printTFTcentered(buf, _tft->color565(255, 255, 255), 3, 0, 80, 320, 40);
    _tft->fillRect(0, 125, 320, 25, bg);
    printTFTcentered("Min 10 min", _tft->color565(230, 230, 230), 2, 0, 125, 320, 25);
}

void OFC_Ui::draw_book_session() {
    clear_screen();
    draw_title((char*)_machine_name.c_str());
    draw_center_background(80, 140, 80);

    printTFTcentered("Book session", _tft->color565(255, 255, 255), 2, 0, 40, 320, 30);
    draw_book_session_values();

    draw_button_left((char*)"+5m | -5m");
    draw_button_right((char*)"X | OK");

    _menu = BOOK_SESSION;
}
