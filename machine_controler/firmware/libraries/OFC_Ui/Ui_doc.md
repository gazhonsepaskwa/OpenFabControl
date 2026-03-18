# How to create new screens ?

1. Create the screen in lopaka.app (or by hand) and put it in a .cpp file in the ./src/screens folder

ex :
#include "../../OFC_Ui.h"

void OFC_Ui::display_scan_card() {

    // lopaka.com generated code (edited to fit need)
    _tft->setTextColor(0xFFFF);
    _tft->setTextSize(2);
    _tft->setTextWrap(false);
    _tft->setCursor(9, 10);
    _tft->print("CNC");
    
    ...
}

2. Write the handler of the screen and put it in the ./src/select_menu/handlers folder
ex: look the handler.template file                   # to-do make clickable the file to open the file

3. create the Menu enum entry, add the case \[menu_name] : with the handler in select_menu.cpp
