#include <Wire.h>
#include "Electroniccats_PN7150.h"
#include <Adafruit_MCP23X17.h>
#define PN7150_IRQ (39)
#define PN7150_VEN (-1) // ignored by lib so i can togle it from the MCP
#define NFC_VEN_MCP (0)
#define PN7150_ADDR (0x28)

Adafruit_MCP23X17 mcp;
Electroniccats_PN7150 nfc(PN7150_IRQ, PN7150_VEN, PN7150_ADDR, PN7150); // creates a global NFC device interface object, attached to pins 7 (IRQ) and 8 (VEN) and using the default I2C address 0x28,specify PN7150 or PN7160 in constructor

// Function prototypes
String getHexRepresentation(const byte* data, const uint32_t numBytes);
void displayCardInfo();

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println("FABMAN hardware reverse engenering");
  Serial.println("POC: NFC chip (PN7150)");

  // MCP part
  if (!Wire.begin())
    {
        Serial.println("Failed to initialize wire");
        while (1);
    }
    if (!mcp.begin_I2C(0x21)) {
        Serial.println("Failed to initialize MCP23017");
        while (1);
    }
  mcp.pinMode(NFC_VEN_MCP, OUTPUT);
  delay(100);
  mcp.digitalWrite(NFC_VEN_MCP, LOW);
  delay(100);
  mcp.digitalWrite(NFC_VEN_MCP, HIGH);
  delay(100);

  // NFC part
  if (nfc.connectNCI()) {  // Wake up the board
    Serial.println("Error while setting up the mode, check connections!");
    while (1);
  }

  if (nfc.configureSettings()) {
    Serial.println("The Configure Settings is failed!");
    while (1);
  }

  // Read/Write mode as default
  if (nfc.configMode()) {  // Set up the configuration mode
    Serial.println("The Configure Mode is failed!!");
    while (1);
  }
  nfc.startDiscovery();  // NCI Discovery mode
  Serial.println("Waiting for an Card ...");
}

void loop() {
  if (nfc.isTagDetected()) {
    displayCardInfo();

    // It can detect multiple cards at the same time if they use the same protocol
    if (nfc.remoteDevice.hasMoreTags()) {
      nfc.activateNextTagDiscovery();
      Serial.println("Multiple cards are detected!");
    }

    Serial.println("Remove the Card");
    nfc.waitForTagRemoval();
    Serial.println("Card removed!");
  }

  Serial.println("Restarting...");
  nfc.reset();
  Serial.println("Waiting for a Card...");
  delay(500);
}

String getHexRepresentation(const byte* data, const uint32_t numBytes) {
  String hexString;

  if (numBytes == 0) {
    hexString = "null";
  }

  for (uint32_t szPos = 0; szPos < numBytes; szPos++) {
    hexString += "0x";
    if (data[szPos] <= 0xF)
      hexString += "0";
    hexString += String(data[szPos] & 0xFF, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1)) {
      hexString += " ";
    }
  }
  return hexString;
}

void displayCardInfo() {  // Funtion in charge to show the card/s in te field
  char tmp[16];

  while (true) {
    switch (nfc.remoteDevice.getProtocol()) {  // Indetify card protocol
      case nfc.protocol.T1T:
      case nfc.protocol.T2T:
      case nfc.protocol.T3T:
      case nfc.protocol.ISODEP:
        Serial.print(" - POLL MODE: Remote activated tag type: ");
        Serial.println(nfc.remoteDevice.getProtocol());
        break;
      case nfc.protocol.ISO15693:
        Serial.println(" - POLL MODE: Remote ISO15693 card activated");
        break;
      case nfc.protocol.MIFARE:
        Serial.println(" - POLL MODE: Remote MIFARE card activated");
        break;
      default:
        Serial.println(" - POLL MODE: Undetermined target");
        return;
    }

    switch (nfc.remoteDevice.getModeTech()) {  // Indetify card technology
      case (nfc.tech.PASSIVE_NFCA):
        Serial.println("\tTechnology: NFC-A");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(), nfc.remoteDevice.getSensResLen()));

        Serial.print("\tNFC ID = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getNFCID(), nfc.remoteDevice.getNFCIDLen()));

        Serial.print("\tSEL RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSelRes(), nfc.remoteDevice.getSelResLen()));

        break;

      case (nfc.tech.PASSIVE_NFCB):
        Serial.println("\tTechnology: NFC-B");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(), nfc.remoteDevice.getSensResLen()));

        Serial.println("\tAttrib RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getAttribRes(), nfc.remoteDevice.getAttribResLen()));

        break;

      case (nfc.tech.PASSIVE_NFCF):
        Serial.println("\tTechnology: NFC-F");
        Serial.print("\tSENS RES = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getSensRes(), nfc.remoteDevice.getSensResLen()));

        Serial.print("\tBitrate = ");
        Serial.println((nfc.remoteDevice.getBitRate() == 1) ? "212" : "424");

        break;

      case (nfc.tech.PASSIVE_NFCV):
        Serial.println("\tTechnology: NFC-V");
        Serial.print("\tID = ");
        Serial.println(getHexRepresentation(nfc.remoteDevice.getID(), sizeof(nfc.remoteDevice.getID())));

        Serial.print("\tAFI = ");
        Serial.println(nfc.remoteDevice.getAFI());

        Serial.print("\tDSF ID = ");
        Serial.println(nfc.remoteDevice.getDSFID(), HEX);
        break;

      default:
        break;
    }

    // It can detect multiple cards at the same time if they are the same technology
    if (nfc.remoteDevice.hasMoreTags()) {
      Serial.println("Multiple cards are detected!");
      if (!nfc.activateNextTagDiscovery()) {
        break;  // Can't activate next tag
      }
    } else {
      break;
    }
  }
}