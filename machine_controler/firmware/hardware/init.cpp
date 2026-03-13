#include "hardware.h"

// Helper to initialize a component with a serial print
static void init_with_serial(hardware& hw, void (hardware::*fct)(), const char* text) {
    Serial.print(text);
    (hw.*fct)();
    Serial.println("OK");
}

// Hardware constructor
hardware::hardware()
    : mcp1(),
      mcp2(),
      nfc(NFC_IRQ, NFC_VEN, NFC_ADDR),
      tft(TFT_CS, TFT_DC, TFT_RST) {
    init_with_serial(*this, &hardware::init_wire,   "Wire init...     ");
    init_with_serial(*this, &hardware::init_mcp1,   "MCP1 init...     ");
    init_with_serial(*this, &hardware::init_mcp2,   "MCP2 init...     ");
    init_with_serial(*this, &hardware::init_tft,    "TFT  init...     ");
    init_with_serial(*this, &hardware::init_relay,  "Relay init...    ");
    init_with_serial(*this, &hardware::init_button, "Button init...   ");
    init_with_serial(*this, &hardware::init_led,    "LED init...      ");
    init_with_serial(*this, &hardware::init_buzzer, "Buzzer init...   ");
    init_with_serial(*this, &hardware::init_nfc,    "NFC init...      ");
}
// Hardware destructor (will never be called since powering off the esp erases the RAM naturally)
// Note : powering off the esp without cleaning is a problem only when writing to flash memory (settings, etc.)
// but it need special hardware to detect it and continue the program until a stopable operation. So i consider it side effect of the power off.
hardware::~hardware() {}

// Wire initialization
void hardware::init_wire() {
    Wire.begin();
}

// MCP1 initialization
void hardware::init_mcp1() {
    this->mcp1 = Adafruit_MCP23X17();
    if (!this->mcp1.begin_I2C(MCP1_ADDR)) {
        Serial.println("KO (address: 0x21)");
        while (1);
    }
}

// MCP2 initialization
void hardware::init_mcp2() {
    if (!this->mcp2.begin_I2C(0x20)) {
        Serial.println("KO (address: 0x20)");
        while (1);
    }
}

// TFT initialization
void hardware::init_tft() {
    this->mcp1.pinMode(TFT_BL, OUTPUT);
    this->mcp1.digitalWrite(TFT_BL, HIGH);
    this->tft.begin();
    this->tft.setSPISpeed(40000000); // SPI max speed (to check again, feel slow)
    this->tft.setRotation(3);
    this->tft.fillScreen(ILI9341_BLACK); // clear
    printTFTcentered("Starting...", this->tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
}

// Relay initialization
void hardware::init_relay() {
    this->mcp1.pinMode(RELAY1, OUTPUT);
    this->mcp1.digitalWrite(RELAY1, LOW);
    this->mcp1.pinMode(RELAY2, OUTPUT);
    this->mcp1.digitalWrite(RELAY2, LOW);
}

// Button initialization
void hardware::init_button() {
    this->mcp2.pinMode(BTN_L, INPUT);
    this->mcp2.pinMode(BTN_R, INPUT);
}

// LED initialization
void hardware::init_led() {
    pinMode(LED, OUTPUT);
}

// Buzzer initialization
void hardware::init_buzzer() {
    this->mcp1.pinMode(BUZZER, OUTPUT);
    this->mcp1.digitalWrite(BUZZER, LOW);
}

// NFC initialization
void hardware::init_nfc() {
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