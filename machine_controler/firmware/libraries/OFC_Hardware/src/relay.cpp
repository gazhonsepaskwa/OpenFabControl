#include "OFC_Hardware.h"

void OFC_Hardware::relay_on(void) {
    this->mcp1.digitalWrite(RELAY1, HIGH);
    this->mcp1.digitalWrite(RELAY2, HIGH);
}

void OFC_Hardware::relay_off(void) {
    this->mcp1.digitalWrite(RELAY1, LOW);
    this->mcp1.digitalWrite(RELAY2, LOW);
}