#include "OFC_Ui.h"
#include "OFC_Network.h" // For next_booking
#include "QRCodeGFX.h"

// Default constructor (only for global declaration)
OFC_Ui::OFC_Ui()
    :   _machine_name(""),
        _tft(0x0),
        _qr(0x0)
{}

// Constructor
OFC_Ui::OFC_Ui(String machine_name, Adafruit_ILI9341* tft, NextBooking* next_booking)
    :   _machine_name(machine_name),
        _tft(tft),
        _qr(new QRCodeGFX(*tft)),
        _menu(INIT),
        _next_booking(next_booking){
    _qr->setScale(2);  // 1 = default size, 2 = double, etc.
}

OFC_Ui::~OFC_Ui() {
    if (_qr) { delete _qr; }
}

void OFC_Ui::begin(String machine_name, Adafruit_ILI9341* tft, NextBooking* next_booking) {
    _machine_name = machine_name;
    _tft = tft;
    _next_booking = next_booking;
    _menu = INIT;
    if (_qr) {
        delete _qr;
        _qr = nullptr;
    }
    if (_tft) {
        _qr = new QRCodeGFX(*_tft);
        _qr->setScale(2);
    }
}

void OFC_Ui::clear_screen() {
    _tft->fillScreen(ILI9341_BLACK);
}

void OFC_Ui::waiting_approval() {
    // generated with lopaka.com
    _tft->setTextColor(0xEBC7);
    _tft->setTextSize(2);
    _tft->setTextWrap(false);
    _tft->setCursor(24, 51);
    _tft->print("Waiting for approval...");
    _tft->drawRect(24, 120, 273, 59, 0xB653);
    _tft->setTextColor(0xFFFF);
    _tft->setTextSize(1);
    _tft->setCursor(89, 138);
    _tft->print("Please go to admin panel");
    _tft->setCursor(92, 154);
    _tft->print("and approve the machine");
}

void OFC_Ui::show_error_screen(const char* msg) {
    clear_screen();
    draw_title(_machine_name.c_str());
    draw_center_background(120, 60, 60);
    printTFTcentered(msg ? msg : "Error", _tft->color565(255, 255, 255), 2, 0, 70, 320, 30);
}

void OFC_Ui::show_setup_error_and_restart(const char* msg) {
    clear_screen();
    draw_title("Setup Error");
    draw_center_background(120, 60, 60);
    printTFTcentered(msg ? msg : "Error", _tft->color565(255, 255, 255), 2, 0, 40, 320, 120);
    printTFTcentered("Restart in 5s...", _tft->color565(255, 255, 255), 2, 0, 120, 320, 30);
    delay(5000);
    ESP.restart();
}

void OFC_Ui::show_setup_ap_instructions(const char* ap_ssid, const char* ap_pass) {
    clear_screen();
    draw_title("Setup");
    draw_center_background(60, 60, 120);
    printTFTcentered("Connect to WiFi:", _tft->color565(255, 255, 255), 2, 0, 25, 320, 22);
    printTFTcentered(ap_ssid ? ap_ssid : "", _tft->color565(255, 255, 255), 2, 0, 47, 320, 22);
    printTFTcentered("Password:", _tft->color565(255, 255, 255), 2, 0, 69, 320, 22);
    printTFTcentered(ap_pass ? ap_pass : "", _tft->color565(255, 255, 255), 2, 0, 91, 320, 22);
    printTFTcentered("Then open browser", _tft->color565(255, 255, 255), 2, 0, 118, 320, 22);
    printTFTcentered("and fill the form", _tft->color565(255, 255, 255), 2, 0, 140, 320, 22);
}

void OFC_Ui::show_setup_connecting_wifi(const char* ssid) {
    clear_screen();
    draw_title("Setup");
    draw_center_background(60, 60, 120);
    printTFTcentered("Connecting to WiFi...", _tft->color565(255, 255, 255), 2, 0, 70, 320, 30);
    printTFTcentered(ssid ? ssid : "", _tft->color565(200, 200, 200), 2, 0, 100, 320, 25);
}

void OFC_Ui::show_setup_registering() {
    clear_screen();
    draw_title("Setup");
    draw_center_background(60, 60, 120);
    printTFTcentered("Registering...", _tft->color565(255, 255, 255), 2, 0, 80, 320, 30);
}

void OFC_Ui::show_setup_complete() {
    clear_screen();
    draw_title("Setup");
    draw_center_background(60, 120, 60);
    printTFTcentered("Setup complete", _tft->color565(255, 255, 255), 2, 0, 80, 320, 30);
    delay(3000);
}

Menu OFC_Ui::get_menu() {
    return _menu;
}
