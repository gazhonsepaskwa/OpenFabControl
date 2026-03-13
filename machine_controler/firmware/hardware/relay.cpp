void hardware::relay_on(void) {
    this->mcp1.digitalWrite(RELAY1, HIGH);
    this->mcp1.digitalWrite(RELAY2, HIGH);
}

void hardware::relay_off(void) {
    this->mcp1.digitalWrite(RELAY1, LOW);
    this->mcp1.digitalWrite(RELAY2, LOW);
}