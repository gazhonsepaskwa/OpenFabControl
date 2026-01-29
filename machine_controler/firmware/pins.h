#pragma once

// MCP23017 (IO_EXPENDERS)
#define MCP1_ADDR 0x21
#define MCP2_ADDR 0x20

// buttons
#define BTN_L 3 // MCP2
#define BTN_R 4 // MCP2

// PN7150 (NFC)
#define NFC_ADDR 0x28
#define NFC_IRQ  39
#define NFC_VEN  0 // MCP1

// TFT [ILI9341 driver] (LCD)
#define TFT_CS  5
#define TFT_DC  4
#define TFT_RST -1
#define TFT_BL  9 // MCP1

// led
#define LED 2

// buzzer
#define BUZZER 14 // MCP1
