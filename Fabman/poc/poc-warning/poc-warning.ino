#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

#define WARNING_LED 2
#define BUZZER 14

Adafruit_MCP23X17 mcp;

void setup() {
    Serial.begin(115200);
    Serial.println("FABMAN hardware reverse engenering");
    Serial.println("POC : attention light");
    pinMode(WARNING_LED, OUTPUT);
    Wire.begin();

    if (!mcp.begin_I2C(0x21)) {
        Serial.println("Failed to initialize MCP23017");
        while (1);
    }

    mcp.pinMode(BUZZER, OUTPUT);
    Serial.println("MCP23017 initialised successfully");
}

void loop() {
    digitalWrite(WARNING_LED, HIGH);
    mcp.digitalWrite(BUZZER, HIGH);
    delay(500);
    digitalWrite(WARNING_LED, LOW);
    mcp.digitalWrite(BUZZER, LOW);
    delay(500);
}
