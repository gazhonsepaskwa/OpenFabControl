#include "../OFC_Ui.h"

void OFC_Ui::update_menu(Event ev) {
    switch (_menu) {
        case INIT:           this->menu_handler_init(ev);           break;
        case SCAN_CARD:      this->menu_handler_scan_card(ev);      break;
        case MACHINE_INFO:   this->menu_handler_machine_info(ev);   break;
        case MACHINE_USAGE:  this->menu_handler_machine_usage(ev);  break;
        case ADD_TIME:       this->menu_handler_add_time(ev);       break;
        case BOOK_SESSION:   this->menu_handler_book_session(ev);   break;
        case CONFIRM_FINISH: this->menu_handler_confirm_finish(ev); break;
    }
}
