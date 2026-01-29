#include <Arduino.h>                // General purpose instructions
#include <Adafruit_GFX.h>           // screen
#include <Adafruit_ILI9341.h>       // screen
#include <cstdint>
#include <QRCodeGFX.h>

#include "screen_utils.h"
#include "firmware.h"

bool setup_process(Preferences& preferences) {
    Serial.println("Settings not found, starting setup mode... (simulated good)");
    preferences.putBool(SETUP_COMPLETED_KEY, true);
    return (true);
}
