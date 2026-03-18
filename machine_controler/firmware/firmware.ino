#include <Arduino.h>                // General purpose instructions
#include <WiFi.h>                   // before PN7150 (NdefMessage.h redefines WIFI_AUTH_OPEN)
#include <cstdio>
#include <Print.h>
#include <HardwareSerial.h>
#include <Preferences.h>

#include "firmware.h"
#include <OFC_Hardware.h>
OFC_Hardware h; // included as extern in other files

Preferences             g_preferences; // included as exterm in other files
bool                    wifi_connection_lost = false;

Session                 current_session = {};
unsigned long           last_tick_ms = 0;
char                    last_scanned_access_key[32] = {0}; // included as extern in other files

// Network
#include <OFC_Network.h>
OFC_Network             network;

// Ui
#include <OFC_Ui.h>
OFC_Ui                  ui;

// Other

void clean_restart(void) {
    g_preferences.end();
    ESP.restart();
}

void setup() {
    Serial.begin(115200);
    Serial.println("╔══════════════════════════════════════════╗");
    Serial.println("║ Program : OFC machine_controler firmware ║");
    Serial.println("║ Version : v beta 1.0                     ║");
    Serial.println("╚══════════════════════════════════════════╝");
    Serial.println("");

    // Open settings namespace (create if not exists)
    g_preferences.begin("settings", false); // false => read & write

    // Hardware init
    h = OFC_Hardware();

    // Ui init
    ui = OFC_Ui(g_preferences.getString(MACHINE_NAME_KEY).c_str(), &h.tft);

    // Setup process if settings not saved
    Serial.print("Setup process... ");
    if (!g_preferences.getBool(SETUP_COMPLETED_KEY)) {
        if (!setup_process(g_preferences)) {
            clean_restart();
        }
    }
    Serial.println("OK");

    // Network init
    // retreive wifi credentials from preferences (access once only)
    // Note: reading from NVS is limmited in number of operations and should be done the least possible
    String sta_ssid = g_preferences.getString(WIFI_STA_SSID_KEY, "");
    String sta_pass = g_preferences.getString(WIFI_STA_PASS_KEY, "");
    network = OFC_Network(sta_ssid, sta_pass, g_preferences.getString(UUID_KEY, ""), g_preferences.getString(MACHINE_API_HOST_KEY, ""));
    network.connectToWifi();
    if (network.isWifiConnected()) {
        network.setTimezone();
    }

    // wait for server to approve the machine
    Serial.print("Approved...       ");
    int first_time = true;
    // if not approved (or server not reachable) display waiting approval screen
    // TODO : disociate the two
    while (!approved_by_admin(g_preferences)) {
        if (first_time) {
            first_time = false;
            ui.clear_screen();
            ui.waiting_approval();
        }
        delay(5000);
    }
    Serial.println("OK");

    // start the interface
    qr.setScale(2);  // 1 = default size, 2 = double, etc.
    api.force_refresh_next_booking();
    select_menu(qr, menu, EVENT_NONE);
}

void loop() {
    // update button state
    bool btnL_state = h.getButtonLeftState();
    bool btnR_state = h.getButtonRightState();

    // If wifi connection lost, reconnect
    network.checkWifiAndReconnect();

    // Periodic refresh next booking info while on scan card screen
    if (menu == SCAN_CARD) {
        if (api.refresh_next_booking_if_needed()) {
            // NextBooking nb = api.get_next_booking(); // ex of how to retreiv the value. i dont exactly know how i'll do, since i have to recreate the select menu.
            ui.update_menu(EVENT_NONE);
        }
    }

    // LEFT BTN EVENT
    if (btnL_state == LOW) {
        unsigned long press_start = millis();
        // wait the button to be released
        while (btnL_state == LOW) {
            btnL_state = h.getButtonLeftState();
        }
        unsigned long duration = millis() - press_start;
        Event ev;
        if ((menu == ADD_TIME || menu == BOOK_SESSION) && duration >= LONG_PRESS_MS) {
            ev = EVENT_BTN_LEFT_LONG;
        } else {
            ev = EVENT_BTN_LEFT;
        }
        select_menu(qr, menu, ev);
    }

    // RIGHT BTN EVENT
    else if (btnR_state == LOW) {
        unsigned long press_start = millis();
        // wait the button to be released
        while (btnR_state == LOW) {
            btnR_state = h.getButtonRightState();
        }
        unsigned long duration = millis() - press_start;
        Event ev;
        if ((menu == ADD_TIME || menu == BOOK_SESSION) && duration >= LONG_PRESS_MS) {
            ev = EVENT_BTN_RIGHT_LONG;
        } else {
            ev = EVENT_BTN_RIGHT;
        }
        select_menu(qr, menu, ev);
    }

    // CARD EVENT
    else if (h.nfc.isTagDetected(20)) {
        if (h.nfc.remoteDevice.hasMoreTags()) {
            Serial.println("todo: error msg for only one tag at the time");
        }
        const unsigned char* uid = h.nfc.remoteDevice.getNFCID();
        unsigned char uid_len = h.nfc.remoteDevice.getNFCIDLen();
        // convert uid to string (hexa)
        if (uid && uid_len > 0 && uid_len <= 15) {
            for (unsigned char i = 0; i < uid_len; i++) {
                snprintf(last_scanned_access_key + i * 2, 4, "%02X", uid[i]);
            }
            last_scanned_access_key[uid_len * 2] = '\0';
        } else {
            last_scanned_access_key[0] = '\0';
        }
        h.nfc.waitForTagRemoval();
        select_menu(qr, menu, EVENT_CARD);
    }
    // Update machine usage time display every second
    else if (menu == MACHINE_USAGE && (millis() - last_tick_ms) >= 1000) {
        update_machine_usage_times();
        last_tick_ms = millis();
    }
    h.nfc.reset();
}
