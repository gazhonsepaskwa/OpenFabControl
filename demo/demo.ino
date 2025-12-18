#include <Arduino.h>                // General purpose instructions
#include <Wire.h>                   // I2C
#include <Adafruit_MCP23X17.h>      // IO Expenders
#include "Electroniccats_PN7150.h"  // NFC
#include <cstdio>
#include <fstream>

#include "pins.h"
#include "demo.h"

// component objects declaration
Adafruit_MCP23X17       mcp1;
Adafruit_MCP23X17       mcp2;
Adafruit_ILI9341        tft(TFT_CS, TFT_DC, TFT_RST); // default VSPI bus
Electroniccats_PN7150   nfc(NFC_IRQ, -1, NFC_ADDR, PN7150); // VIN -> -1 since managed externally

//other
Menu menu = INIT;
QRCodeGFX qr(tft);

void setup() {
    Serial.begin(115200);
    Serial.println("╔══════════════════════════════╗");
    Serial.println("║ Program : OFC_controler demo ║");
    Serial.println("║ Version : 1.0                ║");
    Serial.println("╚══════════════════════════════╝");
    Serial.println("");

    // I2C init
    Serial.print("Wire init...     ");
    if (!Wire.begin()) {
        Serial.println("KO");
        while (1);
    }
    Serial.println("OK");

    // MCPs init
    Serial.print("MCP23017 init... ");
    if (!mcp1.begin_I2C(0x21)) {
        Serial.println("KO (address: 0x21)");
        while (1);
    }
    if (!mcp2.begin_I2C(0x20)) {
        Serial.println("KO (address: 0x20)");
        while (1);
    }
    Serial.println("OK");

    // Buttons init
    Serial.print("Button init...   ");
    mcp2.pinMode(BTN_L, INPUT);
    mcp2.pinMode(BTN_R, INPUT);
    Serial.println("OK");

    // LED
    Serial.print("LED init...      ");
    pinMode(LED, OUTPUT);
    Serial.println("OK");

    // Buzzer
    Serial.print("Buzzer init...   ");
    mcp1.pinMode(BUZZER, OUTPUT);
    Serial.println("OK");

    // Screen init
    Serial.print("TFT init...      ");
    mcp1.pinMode(TFT_BL, OUTPUT);
    mcp1.digitalWrite(TFT_BL, HIGH);
    tft.begin();
    tft.setSPISpeed(40000000); // SPI max speed (to check again, feel slow)
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK); // clear
    Serial.println("OK");
    select_menu(tft, qr, menu, EVENT_ANY);

    // NFC init
    Serial.print("NFC init...      ");
    mcp1.pinMode(NFC_VEN, OUTPUT);
    delay(100);
    mcp1.digitalWrite(NFC_VEN, LOW);
    delay(100);
    mcp1.digitalWrite(NFC_VEN, HIGH);
    delay(100);
    // wake up the board
    if (nfc.connectNCI()) {
        Serial.println("KO: failed to connect to the NFC chip");
        while (1);
    }
    if (nfc.configureSettings()) {
        Serial.println("KO: failed to configure the NFC chip");
        while (1);
    }
    // Read/Write mode as default
    if (nfc.configMode()) {
        Serial.println("KO: failed to configure the mode");
        while (1);
    }
    // NCI Discovery mode
    nfc.startDiscovery();
    Serial.println("OK");
}

void loop() {
    bool btnL_state = mcp2.digitalRead(BTN_L);
    bool btnR_state = mcp2.digitalRead(BTN_R);

    // LEFT BTN EVENT
    if (btnL_state == LOW) {
        // wait the button to be released
        while (btnL_state == LOW) {
            btnL_state = mcp2.digitalRead(BTN_L);
        }
        select_menu(tft, qr, menu, EVENT_BTN_LEFT);
    }

    // RIGHT BTN EVENT
    else if (btnR_state == LOW) {
        // wait the button to be released
        while (btnR_state == LOW) {
            btnR_state = mcp2.digitalRead(BTN_R);
        }
        select_menu(tft, qr, menu, EVENT_BTN_RIGHT);
    }

    // CARD EVENT
    else if (nfc.isTagDetected(20)) {
        if (nfc.remoteDevice.hasMoreTags()) {
            Serial.println("todo: error msg for only one tag at the time");
        }
        Serial.println("todo : msg remove card");
        nfc.waitForTagRemoval();
        // to do : get the card info
        select_menu(tft, qr, menu, EVENT_CARD);
    }
    nfc.reset();
}
