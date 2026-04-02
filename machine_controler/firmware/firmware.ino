#include <Arduino.h>                // General purpose instructions
#include <WiFi.h>                   // before PN7150 (NdefMessage.h redefines WIFI_AUTH_OPEN)
#include <cstdio>
#include <Print.h>
#include <HardwareSerial.h>
#include <Preferences.h>
#include <OFC_Setup_process.h>
#include <OFC_HardwareConfig.h>
#include "preference_keys.h"

// Lib includes
#include <OFC_Hardware.h>
#include <OFC_Network.h>
#include <OFC_Ui.h>

OFC_Hardware g_hardware; // Hardware Lib
OFC_Network  g_network;  // Network Lib
OFC_Ui       g_ui;       // Ui Lib

// Other global
Preferences  g_preferences;
NextBooking  g_next_booking;
char         g_last_scanned_access_key[32] = {0};
Session      g_current_session = {};

// Other
bool wifi_connection_lost = false;

void clean_restart() {
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
    g_hardware.begin();

    // Ui init
    g_next_booking = {}; // temporarily empty next booking because g_network is not initialized yet here (api == nullptr).
    g_ui.begin(
        g_preferences.getString(MACHINE_NAME_KEY).c_str(),
        &g_hardware.tft,
        &g_next_booking
    );

    // Setup process
    OFC_Setup_process setup_process;
    // Setup process if settings not saved
    Serial.print("Setup process... ");
    if (!g_preferences.getBool(SETUP_COMPLETED_KEY)) {
        if (!setup_process.begin(g_preferences, g_ui)) {
            clean_restart();
        }
    }
    Serial.println("OK");

    // Network init
    // retreive wifi credentials from preferences (access once only)
    // Note: reading from NVS is limmited in number of operations and should be done the least possible
    String sta_ssid = g_preferences.getString(WIFI_STA_SSID_KEY, "");
    String sta_pass = g_preferences.getString(WIFI_STA_PASS_KEY, "");
    g_network.begin(sta_ssid, sta_pass, g_preferences.getString(UUID_KEY, ""), g_preferences.getString(MACHINE_API_HOST_KEY, ""));
    g_network.connectToWifi();
    if (g_network.isWifiConnected()) {
        g_network.setTimezone();
    }

    // wait for server to approve the machine
    Serial.print("Approved...       ");
    int first_time = true;
    // if not approved (or server not reachable) display waiting approval screen
    // TODO : disociate the two
    while (true) {
        int http_code = 0;
        bool approved = (g_network.api && g_network.api->is_approved_by_admin(&http_code));
        if (approved) break;

        if (http_code == 404) {
            Serial.println("KO: machine not found");
            g_ui.clear_screen();
            g_ui.waiting_approval();
            while (true) {
                if (g_hardware.mcp2.digitalRead(BTN_L) == LOW || g_hardware.mcp2.digitalRead(BTN_R) == LOW) {
                    g_preferences.clear();
                    ESP.restart();
                }
                delay(100);
            }
        }
        if (first_time) {
            first_time = false;
            g_ui.clear_screen();
            g_ui.waiting_approval();
        }
        delay(5000);
    }
    Serial.println("OK");

    // start the interface
    g_network.api->force_refresh_next_booking();
    g_ui.update_menu(EVENT_NONE);
}

void loop() {
    ////////////
    // checks //
    ////////////

    // If wifi connection lost, reconnect
    static bool was_wifi_connection_lost = false;
    g_network.checkWifiAndReconnect();
    // If we just reconnected, resync time and refresh next booking.
    if (was_wifi_connection_lost && !wifi_connection_lost && g_network.isWifiConnected()) {
        g_network.setTimezone();
        if (g_network.api) {
            g_network.api->force_refresh_next_booking();
            g_next_booking = g_network.api->get_next_booking();
        }
        if (g_ui.get_menu() == SCAN_CARD) {
            g_ui.redraw_scan_card();
        }
    }
    was_wifi_connection_lost = wifi_connection_lost;

    // Periodic refresh next booking info while on scan card screen
    if (g_ui.get_menu() == SCAN_CARD) {
        if (g_network.api && g_network.api->refresh_next_booking_if_needed()) {
            g_next_booking = g_network.api->get_next_booking();
            // Booking changed -> redraw scan card screen.
            g_ui.redraw_scan_card();
        }
    }

    // Update machine usage time display every second
    else if (g_ui.get_menu() == MACHINE_USAGE) {
        g_ui.update_machine_usage_times();
    }

    /////////////////////
    // updates (EVENT) //
    /////////////////////

    // update button state
    bool btnL_state = g_hardware.getButtonLeftState();
    bool btnR_state = g_hardware.getButtonRightState();

    // LEFT BTN EVENT
    if (btnL_state == LOW) {
        // g_harware.getButtonLeftPressTime(); // TODO move to a fct maybe
        unsigned long press_start = millis();
        // wait the button to be released
        while (btnL_state == LOW) {
            // maybe do something on the UI so the user now he need unpress the button
            btnL_state = g_hardware.getButtonLeftState();
            delay(1); // avoid watchdog reset while waiting
        }
        unsigned long duration = millis() - press_start;
        // up to here

        if ((g_ui.get_menu() == ADD_TIME || g_ui.get_menu() == BOOK_SESSION) && duration >= OFC_LONG_PRESS_MS) {
            g_ui.update_menu(EVENT_BTN_LEFT_LONG);
        } else {
            g_ui.update_menu(EVENT_BTN_LEFT);
        }
        // or maybe here
    }

    // RIGHT BTN EVENT
    else if (btnR_state == LOW) {
        // same here as over
        unsigned long press_start = millis();
        // wait the button to be released
        while (btnR_state == LOW) {
            btnR_state = g_hardware.getButtonRightState();
            delay(1); // avoid watchdog reset while waiting
        }
        unsigned long duration = millis() - press_start;

        if ((g_ui.get_menu() == ADD_TIME || g_ui.get_menu() == BOOK_SESSION) && duration >= OFC_LONG_PRESS_MS) {
            g_ui.update_menu(EVENT_BTN_RIGHT_LONG);
        } else {
            g_ui.update_menu(EVENT_BTN_RIGHT);
        }
    }

    // CARD EVENT
    else if (g_hardware.nfc.isTagDetected(20)) {
        if (g_hardware.nfc.remoteDevice.hasMoreTags()) {
            Serial.println("todo: error msg for only one tag at the time"); // TODO
        }
        const unsigned char* uid = g_hardware.nfc.remoteDevice.getNFCID();
        unsigned char uid_len = g_hardware.nfc.remoteDevice.getNFCIDLen();
        // convert uid to string (hexa)
        if (uid && uid_len > 0 && uid_len <= 15) {
            for (unsigned char i = 0; i < uid_len; i++) {
                snprintf(g_last_scanned_access_key + i * 2, 4, "%02X", uid[i]);
            }
            g_last_scanned_access_key[uid_len * 2] = '\0'; // TODO why * 2 ?
        } else {
            g_last_scanned_access_key[0] = '\0';
        }
        g_hardware.nfc.waitForTagRemoval();
        g_ui.update_menu(EVENT_CARD);
    }

    g_hardware.nfc.reset();
}

