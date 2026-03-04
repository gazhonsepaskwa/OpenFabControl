#include <Arduino.h>
#include "pins.h"
#include "utils.h"

extern Adafruit_MCP23X17 mcp2;

void relay_on() {
    mcp1.digitalWrite(RELAY1, HIGH);
    mcp1.digitalWrite(RELAY2, HIGH);
}

void relay_off() {
    mcp1.digitalWrite(RELAY1, LOW);
    mcp1.digitalWrite(RELAY2, LOW);
}