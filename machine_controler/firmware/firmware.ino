#include <Arduino.h>                // General purpose instructions
#include <Wire.h>                   // I2C
#include <WiFi.h>                   // before PN7150 (NdefMessage.h redefines WIFI_AUTH_OPEN)
#include <time.h>
#include <Adafruit_MCP23X17.h>      // IO Expenders
#include "Electroniccats_PN7150.h"  // NFC
#include <cstdio>
#include <fstream>
#include <Preferences.h>

#include "pins.h"
#include "firmware.h"
#include "screen_utils.h"
#include "utils.h"

// component objects declaration
Adafruit_MCP23X17       mcp1;
Adafruit_MCP23X17       mcp2;
Adafruit_ILI9341        tft(TFT_CS, TFT_DC, TFT_RST); // default VSPI bus
Electroniccats_PN7150   nfc(NFC_IRQ, -1, NFC_ADDR, PN7150); // VIN -> -1 since managed externally

// Settings
Preferences             preferences;

// Session (from start_session API)
Session                 current_session = {};
unsigned long           last_tick_ms = 0;
char                    last_scanned_access_key[32] = {0};

// Next booking info
NextBooking             next_booking = {};
unsigned long           last_next_booking_refresh_ms = 0;

// WiFi status (set in loop when disconnected; used to show "SA" (stand alone) on screen)
bool                    wifi_connection_lost = false;
unsigned long           last_wifi_check_ms = 0;
unsigned long           last_wifi_reconnect_ms = 0;

// Other
Menu menu = INIT;  // enum
QRCodeGFX qr(tft);

void setup() {
    Serial.begin(115200);
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║ Program : OFC machine_controler firmware ║");
    Serial.println("║ Version : v beta 1.0                     ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println("");

    // I2C init
    Serial.print("Wire init...     ");
    if (!Wire.begin()) {
        Serial.println("KO");
        while (1);
    }
    Serial.println("OK");

    // MCPs init
    Serial.print("MCP23017 init... ");
    if (!mcp1.begin_I2C(0x21)) {
        Serial.println("KO (address: 0x21)");
        while (1);
    }
    if (!mcp2.begin_I2C(0x20)) {
        Serial.println("KO (address: 0x20)");
        while (1);
    }
    Serial.println("OK");

    // Relay init
    Serial.print("Relay init...    ");
    mcp1.pinMode(RELAY1, OUTPUT);
    mcp1.digitalWrite(RELAY1, LOW);
    mcp1.pinMode(RELAY2, OUTPUT);
    mcp1.digitalWrite(RELAY2, LOW);
    Serial.println("OK");

    // Buttons init
    Serial.print("Button init...   ");
    mcp2.pinMode(BTN_L, INPUT);
    mcp2.pinMode(BTN_R, INPUT);
    Serial.println("OK");

    // LED
    Serial.print("LED init...      ");
    pinMode(LED, OUTPUT);
    Serial.println("OK");

    // Buzzer
    Serial.print("Buzzer init...   ");
    mcp1.pinMode(BUZZER, OUTPUT);
    mcp1.digitalWrite(BUZZER, LOW);
    Serial.println("OK");

    // Screen init
    Serial.print("TFT init...      ");
    mcp1.pinMode(TFT_BL, OUTPUT);
    mcp1.digitalWrite(TFT_BL, HIGH);
    tft.begin();
    tft.setSPISpeed(40000000); // SPI max speed (to check again, feel slow)
    tft.setRotation(3);
    tft.fillScreen(ILI9341_BLACK); // clear
    Serial.println("OK");

    // NFC init
    Serial.print("NFC init...      ");
    mcp1.pinMode(NFC_VEN, OUTPUT);
    delay(100);
    mcp1.digitalWrite(NFC_VEN, LOW);
    delay(100);
    mcp1.digitalWrite(NFC_VEN, HIGH);
    delay(100);
    // wake up the board
    if (nfc.connectNCI()) {
        Serial.println("KO: failed to connect to the NFC chip");
        while (1);
    }
    if (nfc.configureSettings()) {
        Serial.println("KO: failed to configure the NFC chip");
        while (1);
    }
    // Read/Write mode as default
    if (nfc.configMode()) {
        Serial.println("KO: failed to configure the NFC mode");
        while (1);
    }
    // NCI Discovery mode
    nfc.startDiscovery();
    Serial.println("OK");

    // Open settings namespace
    preferences.begin("settings", false); // false => read & write

    // Setup process if settings not saved
    Serial.print("Setup process... ");
    if (!preferences.getBool(SETUP_COMPLETED_KEY)) {
        if (!setup_process(preferences)) {
            preferences.end();
            ESP.restart();
        }
    }
    Serial.println("OK");

    // Connect to WiFi
    String sta_ssid = preferences.getString(WIFI_STA_SSID_KEY, "");
    String sta_pass = preferences.getString(WIFI_STA_PASS_KEY, "");
    if (sta_ssid.length() > 0) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - start) < 20000)
            delay(200);

        // set timezone
        configTime(0, 0, "pool.ntp.org");
        setenv("TZ", TZ_STRING, 1);
        tzset();

        // wait for NTP sync (needed for draw_scan_card to compute has_booking_today)
        struct tm timeinfo;
        int ntp_retries = 0;
        while (!getLocalTime(&timeinfo) && ntp_retries++ < 30) {
            delay(500);
        }
        if (!getLocalTime(&timeinfo)) {
            Serial.println("WARN: NTP sync failed, time-based display may be wrong");
        }
    }

    // wait for server to approve the machine
    Serial.print("Approved...       ");
    int first_time = true;
    while (!approved_by_admin(preferences)) {
        if (first_time) {
            first_time = false;
            clear_screen();
            draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
            draw_center_background(60, 60, 120);
            printTFTcentered("Waiting for approval...", tft.color565(255, 255, 255), 2, 0, 70, 320, 30);
            printTFTcentered("Please go to admin panel", tft.color565(255, 255, 255), 2, 0, 100, 320, 30);
            printTFTcentered("and approve the machine.", tft.color565(255, 255, 255), 2, 0, 130, 320, 30);
        }
        delay(5000);
    }
    Serial.println("OK");

    // start the interface
    force_refresh_next_booking();
    Serial.println("Next booking: ");
    Serial.println(next_booking.has_booking);
    Serial.println(next_booking.start_unix);
    Serial.println(next_booking.end_unix);
    Serial.println(next_booking.user_name);
    select_menu(qr, menu, EVENT_ANY);
}

void refresh_next_booking_if_needed(void) {
    unsigned long now_ms = millis();
    if ((now_ms - last_next_booking_refresh_ms) < NEXT_BOOKING_REFRESH_INTERVAL_MS) {
        return;
    }
    if (fetch_next_booking(&next_booking)) {
        last_next_booking_refresh_ms = now_ms;
        // If we are on the scan card screen, redraw it to reflect updated booking info
        if (menu == SCAN_CARD) {
            draw_scan_card(qr, menu);
        }
    }
}

void force_refresh_next_booking(void) {
    if (fetch_next_booking(&next_booking)) {
        last_next_booking_refresh_ms = millis();
    }
}

void check_wifi_and_reconnect(void) {
    unsigned long now_ms = millis();
    if (now_ms - last_wifi_check_ms < WIFI_CHECK_INTERVAL_MS)
        return;
    last_wifi_check_ms = now_ms;

    if (WiFi.status() != WL_CONNECTED) {
        if (!wifi_connection_lost) {
            wifi_connection_lost = true;
            draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
        }
        if (now_ms - last_wifi_reconnect_ms >= WIFI_RECONNECT_INTERVAL_MS) {
            last_wifi_reconnect_ms = now_ms;
            String sta_ssid = preferences.getString(WIFI_STA_SSID_KEY, "");
            String sta_pass = preferences.getString(WIFI_STA_PASS_KEY, "");
            if (sta_ssid.length() > 0) {
                WiFi.disconnect();
                WiFi.begin(sta_ssid.c_str(), sta_pass.c_str());
            }
        }
    } else {
        if (wifi_connection_lost) {
            wifi_connection_lost = false;
            draw_title((char*)preferences.getString(MACHINE_NAME_KEY).c_str());
        }
    }
}

void loop() {
    bool btnL_state = mcp2.digitalRead(BTN_L);
    bool btnR_state = mcp2.digitalRead(BTN_R);

    check_wifi_and_reconnect();

    // Periodic refresh of next booking info while on scan card screen
    if (menu == SCAN_CARD) {
        refresh_next_booking_if_needed();
    }

    // LEFT BTN EVENT
    if (btnL_state == LOW) {
        // wait the button to be released
        while (btnL_state == LOW) {
            btnL_state = mcp2.digitalRead(BTN_L);
        }
        select_menu(qr, menu, EVENT_BTN_LEFT);
    }

    // RIGHT BTN EVENT
    else if (btnR_state == LOW) {
        // wait the button to be released
        while (btnR_state == LOW) {
            btnR_state = mcp2.digitalRead(BTN_R);
        }
        select_menu(qr, menu, EVENT_BTN_RIGHT);
    }

    // CARD EVENT
    else if (nfc.isTagDetected(20)) {
        if (nfc.remoteDevice.hasMoreTags()) {
            Serial.println("todo: error msg for only one tag at the time");
        }
        const unsigned char* uid = nfc.remoteDevice.getNFCID();
        unsigned char uid_len = nfc.remoteDevice.getNFCIDLen();
        // convert uid to string (hexa)
        if (uid && uid_len > 0 && uid_len <= 15) {
            for (unsigned char i = 0; i < uid_len; i++) {
                snprintf(last_scanned_access_key + i * 2, 4, "%02X", uid[i]);
            }
            last_scanned_access_key[uid_len * 2] = '\0';
        } else {
            last_scanned_access_key[0] = '\0';
        }
        nfc.waitForTagRemoval();
        select_menu(qr, menu, EVENT_CARD);
    }
    // Update machine usage time display every second
    else if (menu == MACHINE_USAGE && (millis() - last_tick_ms) >= 1000) {
        update_machine_usage_times();
        last_tick_ms = millis();
    }
    nfc.reset();
}
