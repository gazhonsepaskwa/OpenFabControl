#include <Arduino.h>                // General purpose instructions
#include <Wire.h>                   // I2C
#include <Adafruit_MCP23X17.h>      // IO Expenders
#include "Electroniccats_PN7150.h"  // NFC
#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
#include <cstdint>
#include <cstdio>
#include <fstream>

#include "pins.h"

// component objects declaration
Adafruit_MCP23X17       mcp1;
Adafruit_MCP23X17       mcp2;
Adafruit_ILI9341        tft(TFT_CS, TFT_DC, TFT_RST); // default VSPI bus
Electroniccats_PN7150   nfc(NFC_IRQ, -1, NFC_ADDR, PN7150); // VIN -> -1 since managed externally

void printTFT(char* text, int16_t x, int16_t y, uint16_t color, uint8_t size) {
    tft.setTextColor(color);
    tft.setTextSize(size);
    tft.setCursor(x, y);
    tft.print(text);
}

void draw_title(char* msg) {
    printTFT(msg, 5, 3, tft.color565(255, 255, 255), 2);
}

void clear_screen(Adafruit_ILI9341& tft) {
    tft.fillScreen(ILI9341_BLACK);
}

void setup() {
    Serial.begin(115200);
    Serial.println("╔══════════════════════════════╗");
    Serial.println("║ Program : scan card          ║");
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

    // Screen init
    Serial.print("TFT init...      ");
    mcp1.pinMode(TFT_BL, OUTPUT);
    mcp1.digitalWrite(TFT_BL, HIGH);
    tft.begin();
    tft.setSPISpeed(40000000); // SPI max speed (to check again, feel slow)
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK); // clear
    Serial.println("OK");

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
    // CARD EVENT
    if (nfc.isTagDetected(20)) {
        if (nfc.remoteDevice.hasMoreTags()) {
            Serial.println("Error: Only one tag at a time supported.");
            delay(500);
            return;
        }

        const unsigned char* uid = nfc.remoteDevice.getNFCID();
        unsigned char uid_len = nfc.remoteDevice.getNFCIDLen();

        if (uid && uid_len > 0 && uid_len <= 15) {
            // Allocate buffer for UID string (each byte = 2 hex chars + null terminator)
            char key[32]; // Max 15 bytes * 2 chars + 1 null = 31 chars + null
            for (unsigned char i = 0; i < uid_len; i++) {
                snprintf(&key[i * 2], 3, "%02X", uid[i]); // 3 = max chars to write (including null)
            }
            key[uid_len * 2] = '\0'; // Ensure null termination

            // Display UID on screen
            clear_screen(tft);
            draw_title(key);
            Serial.println(key);

            // Wait until tag is removed
            nfc.waitForTagRemoval();

            // Clear screen after removal
            clear_screen(tft);
        } else {
            Serial.println("Invalid UID.");
        }
    } else {
        delay(500);
    }

    nfc.reset(); // Not necessary here
}


