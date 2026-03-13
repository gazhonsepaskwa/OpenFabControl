#pragma once

// hardware components libraries
#include <Adafruit_MCP23X17.h>      // IO Expenders
#include <Electroniccats_PN7150.h>  // NFC
#include <Adafruit_ILI9341.h>       // TFT

// hardware components pins definitions
#include "hardware/pins.h"

// hardware class definition
class hardware {
    public: // constructor & destructor
        hardware();
        ~hardware();

    private: // methods
        void init_wire();
        void init_mcp1();
        void init_mcp2();
        void init_tft();
        void init_relay();
        void init_button();
        void init_led();
        void init_buzzer();
        void init_nfc();
    
    public: // attributes
        Adafruit_MCP23X17 mcp1;
        Adafruit_MCP23X17 mcp2;
        Electroniccats_PN7150 nfc;
        Adafruit_ILI9341 tft;

    public: // methods
        void relay_on(void);
        void relay_off(void);
};