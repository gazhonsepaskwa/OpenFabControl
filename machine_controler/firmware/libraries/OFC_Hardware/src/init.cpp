#include "OFC_Hardware.h"
#include "../../../theme.h"
#include <Wire.h>

// Helper to initialize a component with a serial print
static void init_with_serial(OFC_Hardware& hw, void (OFC_Hardware::*fct)(), const char* text) {
    Serial.print(text);
    (hw.*fct)();
    Serial.println("OK");
}

// Hardware constructor
// Note: keep it side-effect free. Call begin() from setup() after Serial.begin().
OFC_Hardware::OFC_Hardware()
    : mcp1(),
      mcp2(),
      // Important: on this hardware, NFC_VEN is driven via MCP1 (not an ESP32 GPIO).
      // Using NFC_VEN as a GPIO (e.g. 0) can interfere with board bootstrap / power rails.
      nfc(NFC_IRQ, -1, NFC_ADDR),
      tft(TFT_CS, TFT_DC, TFT_RST) {}
// Hardware destructor (will never be called since powering off the esp erases the RAM naturally)
// Note : powering off the esp without cleaning is a problem only when writing to flash memory (settings, etc.)
// but it need special hardware to detect it and continue the program until a stopable operation. So i consider it side effect of the power off.
OFC_Hardware::~OFC_Hardware() {}

void OFC_Hardware::begin() {
    init_with_serial(*this, &OFC_Hardware::init_wire,   "Wire init...     ");
    init_with_serial(*this, &OFC_Hardware::init_mcp1,   "MCP1 init...     ");
    init_with_serial(*this, &OFC_Hardware::init_mcp2,   "MCP2 init...     ");
    init_with_serial(*this, &OFC_Hardware::init_tft,    "TFT  init...     ");
    init_with_serial(*this, &OFC_Hardware::init_relay,  "Relay init...    ");
    init_with_serial(*this, &OFC_Hardware::init_button, "Button init...   ");
    init_with_serial(*this, &OFC_Hardware::init_led,    "LED init...      ");
    init_with_serial(*this, &OFC_Hardware::init_buzzer, "Buzzer init...   ");
    init_with_serial(*this, &OFC_Hardware::init_nfc,    "NFC init...      ");
}

// Wire initialization
void OFC_Hardware::init_wire() {
    Wire.begin();
}

// MCP1 initialization
void OFC_Hardware::init_mcp1() {
    if (!this->mcp1.begin_I2C(MCP1_ADDR)) {
        Serial.println("KO (address: 0x21)");
        while (1);
    }
}

// MCP2 initialization
void OFC_Hardware::init_mcp2() {
    if (!this->mcp2.begin_I2C(MCP2_ADDR)) {
        Serial.println("KO (address: 0x20)");
        while (1);
    }
}

// TFT initialization
void OFC_Hardware::init_tft() {
    this->mcp1.pinMode(TFT_BL, OUTPUT);
    this->mcp1.digitalWrite(TFT_BL, HIGH);
    this->tft.begin();
    this->tft.setSPISpeed(40000000); // SPI max speed (to check again, feel slow)
    this->tft.setRotation(3);
    // start screen
    this->tft.fillScreen(BACKGROUND_COLOR); // clear
    this->tft.setTextColor(TITLE_COLOR);
    this->tft.setTextSize(2);
    this->tft.setTextWrap(false);
    this->tft.setCursor(113, 96);
    this->tft.print("Starting");
    this->tft.setTextColor(TEXT_COLOR);
    this->tft.setTextSize(1);
    this->tft.setCursor(128, 123);
    this->tft.print("please wait");
}

// Relay initialization
void OFC_Hardware::init_relay() {
    this->mcp1.pinMode(RELAY1, OUTPUT);
    this->mcp1.digitalWrite(RELAY1, LOW);
    this->mcp1.pinMode(RELAY2, OUTPUT);
    this->mcp1.digitalWrite(RELAY2, LOW);
}

// Button initialization
void OFC_Hardware::init_button() {
    this->mcp2.pinMode(BTN_L, INPUT);
    this->mcp2.pinMode(BTN_R, INPUT);
    this->mcp2.pinMode(MCP2_GPA1, INPUT);
}

// LED initialization
void OFC_Hardware::init_led() {
    pinMode(LED, OUTPUT);
}

// Buzzer initialization
void OFC_Hardware::init_buzzer() {
    this->mcp1.pinMode(BUZZER, OUTPUT);
    this->mcp1.digitalWrite(BUZZER, LOW);
}

// NFC initialization
void OFC_Hardware::init_nfc() {
    this->mcp1.pinMode(NFC_VEN, OUTPUT);
    delay(100); // TODO: check if this is needed
    this->mcp1.digitalWrite(NFC_VEN, LOW);
    delay(100);
    this->mcp1.digitalWrite(NFC_VEN, HIGH);
    delay(100);

    // wake up the board
    if (this->nfc.connectNCI()) {
        Serial.println("KO: failed to connect to the NFC controler interface");
        while (1);
    }

    if (this->nfc.configureSettings()) {
        Serial.println("KO: failed to configure the NFC controller interface");
        while (1);
    }
    // Read/Write mode as default
    if (this->nfc.configMode()) {
        Serial.println("KO: failed to configure the NFC controller interface mode");
        while (1);
    }
    // NCI (NFC controller interface) Discovery mode
    this->nfc.startDiscovery();
}