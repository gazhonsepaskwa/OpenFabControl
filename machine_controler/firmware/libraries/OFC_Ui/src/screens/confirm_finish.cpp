#include "OFC_Ui.h"
void OFC_Ui::draw_confirm_finish() {
    clear_screen();
    draw_title((char*)_machine_name.c_str());
    draw_button_left("<- Back");
    draw_button_right("Confirm", 255, 100, 100);
    printTFTcentered("Finish session early ?", _tft->color565(255, 100, 100), 2, 0, 23, 320, 167);
    _menu = CONFIRM_FINISH;
}
