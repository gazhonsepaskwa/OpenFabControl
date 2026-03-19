#include "../../OFC_Ui.h"

void OFC_Ui::menu_handler_machine_info(Event ev) {
    switch (ev) {
        case Event::EVENT_BTN_LEFT :
            this->draw_scan_card();
    }
}
