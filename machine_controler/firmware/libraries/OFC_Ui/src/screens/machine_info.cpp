#include "OFC_Ui.h"

void OFC_Ui::draw_machine_info() {
    clear_screen();
    draw_title((char*)_machine_name.c_str());
    draw_button_left("<- Back");
    printTFTcentered( "User Manual",   _tft->color565(0, 0, 0), 2, 0, 150, 160, 30);
    _qr->draw("https://www.youtube.com/watch?v=dQw4w9WgXcQ", 120, 75);
    _menu = MACHINE_INFO;
}
