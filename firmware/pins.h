#pragma once

// MCP23017 (IO_EXPENDERS)
#define MCP1_ADDR 0x21
#define MCP2_ADDR 0x20

// PN7150 (NFC)
#define NFC_ADDR 0x28
#define NFC_IRQ  39
#define NFC_VEN  0 // MCP1

// TFT [ILI9341 driver] (LCD)
#define TFT_CS  5
#define TFT_DC  4
#define TFT_RST -1
#define TFT_BL  9 // MCP1
