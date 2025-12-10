#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_MCP23X17.h>

// === Pins ===
#define TFT_CS    5
#define TFT_DC    4
#define TFT_RST   -1

#define BACKLIGHT_PIN 9

Adafruit_MCP23X17 mcp;
Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST); // default VSPI bus

void setup() {
    Serial.begin(115200);
    Serial.println("FABMAN hardware reverse engenering");
    Serial.println("POC : screen");

    // MCP23017
    Wire.begin();
    if (!mcp.begin_I2C(0x21)) {
        Serial.println("Failed to initialize MCP23017");
        while (1);
    }
    mcp.pinMode(BACKLIGHT_PIN, OUTPUT);
    // backlight
    mcp.digitalWrite(BACKLIGHT_PIN, HIGH);

    // TFT init on default VSPI
    tft.begin();
    tft.setSPISpeed(40000000);  // SPI max speed
    tft.setRotation(1); // to check
    tft.fillScreen(ILI9341_BLACK); // flush

    Serial.println("TFT initialized successfully");
}

void loop() {
    // Test colors
    tft.fillScreen(ILI9341_RED);
    delay(1000);
    tft.fillScreen(ILI9341_GREEN);
    delay(1000);
    tft.fillScreen(ILI9341_BLUE);
    delay(1000);
}
