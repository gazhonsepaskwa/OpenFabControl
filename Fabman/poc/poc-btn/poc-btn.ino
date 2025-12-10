#include <Wire.h>
#include <Adafruit_MCP23X17.h>

#define NO_BUTTON 3
#define YES_BUTTON 4

// MCP23017
Adafruit_MCP23X17 mcp;

void setup() {
    Serial.begin(115200);
    Serial.println("FABMAN hardware reverse engenering");
    Serial.println("POC : buttons");
    Wire.begin();

    if (!mcp.begin_I2C(0x20)) {
        Serial.println("Failed to initialize MCP23017");
        while (1);
    }
    mcp.pinMode(NO_BUTTON, INPUT);
    mcp.pinMode(YES_BUTTON, INPUT);
    Serial.println("MCP23017 initialised successfully");
}

void loop() {
    int NoButtonState = mcp.digitalRead(NO_BUTTON);
    int YesButtonState = mcp.digitalRead(YES_BUTTON);
    if (NoButtonState == LOW)  { Serial.println("NO_BTN pressed");  }
    if (YesButtonState == LOW) { Serial.println("YES_BTN pressed"); }
}
