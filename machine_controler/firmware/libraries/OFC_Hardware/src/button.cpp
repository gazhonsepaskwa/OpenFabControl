#include "OFC_Hardware.h"

bool OFC_Hardware::getButtonLeftState() {
    return this->mcp2.digitalRead(BTN_L);
}

bool OFC_Hardware::getButtonRightState() {
    return this->mcp2.digitalRead(BTN_R);
}