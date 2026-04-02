#include <OFC_Ui.h>

// init load the scan_card screen
void OFC_Ui::menu_handler_init(Event ev) {
    (void) ev; // not used for this handler
    draw_scan_card();
}
