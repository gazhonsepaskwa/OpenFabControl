#include <OFC_Ui.h>
#include "OFC_Hardware.h"
#include <Arduino.h>
#include <Preferences.h>
#include <Update.h>

#include "mbedtls/sha256.h"

#include <OFC_Firmware.h>

// global vars
extern char         g_last_scanned_access_key[32];
extern Preferences  g_preferences;
extern OFC_Hardware g_hardware;
extern OFC_Network  g_network;
extern Session      g_current_session;

// init load the scan_card screen
void OFC_Ui::menu_handler_scan_card(Event ev) {
    switch (ev) {
        case Event::EVENT_BTN_LEFT:
            draw_machine_info();
            break;

        case Event::EVENT_BTN_RIGHT: {
            if (!g_network.isWifiConnected()) {
                show_error_screen("WiFi disconnected");
                delay(2500);
                draw_scan_card();
                break;
            }

            clear_screen();
            draw_title(_machine_name.c_str());
            draw_center_background(60, 60, 120);
            printTFTcentered("Checking update...", _tft->color565(255, 255, 255), 2, 0, 80, 320, 30);

            char remote_ver[24] = {0};
            char errbuf[64] = {0};
            if (!g_network.api || !g_network.api->get_last_firmware_version(remote_ver, sizeof(remote_ver), errbuf, sizeof(errbuf))) {
                show_error_screen(errbuf[0] ? errbuf : "Update check failed");
                delay(3000);
                draw_scan_card();
                break;
            }

            const int cmp = ofc_compare_versions(OFC_FIRMWARE_VERSION, remote_ver);
            if (cmp >= 0) {
                clear_screen();
                draw_title(_machine_name.c_str());
                draw_center_background(60, 120, 60);
                printTFTcentered("Up to date", _tft->color565(255, 255, 255), 2, 0, 80, 320, 30);
                delay(2000);
                draw_scan_card();
                break;
            }

            clear_screen();
            draw_title(_machine_name.c_str());
            draw_center_background(255, 165, 0);
            printTFTcentered("Update available", _tft->color565(0, 0, 0), 2, 0, 70, 320, 30);
            printTFTcentered(remote_ver, _tft->color565(0, 0, 0), 2, 0, 100, 320, 30);

            // Fetch expected checksum
            char expected_sha256[72] = {0};
            if (!g_network.api->get_firmware_checksum_sha256(expected_sha256, sizeof(expected_sha256), errbuf, sizeof(errbuf))) {
                show_error_screen(errbuf[0] ? errbuf : "Checksum failed");
                delay(3000);
                draw_scan_card();
                break;
            }

            Serial.printf("[OTA] Current=%s Remote=%s\n", OFC_FIRMWARE_VERSION, remote_ver);
            Serial.printf("[OTA] Expected sha256=%s\n", expected_sha256);

            clear_screen();
            draw_title(_machine_name.c_str());
            draw_center_background(60, 60, 120);
            printTFTcentered("Downloading...", _tft->color565(255, 255, 255), 2, 0, 70, 320, 30);

            // Download firmware binary
            HTTPClient http;
            WiFiClientSecure client;
            client.setInsecure(); // keep behavior consistent with existing API client
            // Use the same NVS key as setup process ("api_host"). Kept as literal to avoid sketch header deps.
            String fw_url = "https://" + g_preferences.getString("api_host", "") + "/machine-api/firmware";
            if (fw_url.indexOf("https://") != 0 || fw_url.length() <= strlen("https://")) {
                show_error_screen("API host missing");
                delay(3000);
                draw_scan_card();
                break;
            }

            http.begin(client, fw_url);
            http.setTimeout(30000);
            http.setConnectTimeout(30000);
            int code = http.GET();
            if (code < 200 || code >= 300) {
                snprintf(errbuf, sizeof(errbuf), (code < 0) ? "Network error" : "HTTP %d", code);
                http.end();
                show_error_screen(errbuf);
                delay(3000);
                draw_scan_card();
                break;
            }

            const int total_len = http.getSize();
            if (total_len <= 0) {
                http.end();
                show_error_screen("Missing Content-Length");
                delay(3500);
                draw_scan_card();
                break;
            }

            if (!Update.begin((size_t)total_len, U_FLASH)) {
                http.end();
                show_error_screen("OTA not supported");
                delay(3500);
                draw_scan_card();
                break;
            }

            mbedtls_sha256_context sha;
            mbedtls_sha256_init(&sha);
            mbedtls_sha256_starts(&sha, 0);

            WiFiClient* stream = http.getStreamPtr();
            uint8_t buf[1024];
            int written_total = 0;
            unsigned long last_ui_ms = 0;

            while (http.connected() && written_total < total_len) {
                size_t avail = stream->available();
                if (avail == 0) {
                    delay(1);
                    continue;
                }

                size_t to_read = avail;
                if (to_read > sizeof(buf)) to_read = sizeof(buf);
                if ((int)to_read > (total_len - written_total)) to_read = (size_t)(total_len - written_total);

                int r = stream->readBytes(buf, to_read);
                if (r <= 0) {
                    snprintf(errbuf, sizeof(errbuf), "Download read failed");
                    break;
                }

                mbedtls_sha256_update(&sha, buf, (size_t)r);

                size_t w = Update.write(buf, (size_t)r);
                if (w != (size_t)r) {
                    snprintf(errbuf, sizeof(errbuf), "Flash write failed");
                    break;
                }
                written_total += (int)w;

                unsigned long now = millis();
                if (now - last_ui_ms > 700) {
                    last_ui_ms = now;
                    char pct[24];
                    int p = (int)((written_total * 100LL) / total_len);
                    snprintf(pct, sizeof(pct), "%d%%", p);
                    printTFTcentered(pct, _tft->color565(230, 230, 230), 2, 0, 100, 320, 30);
                }
            }

            http.end();

            uint8_t sha_bin[32];
            mbedtls_sha256_finish(&sha, sha_bin);
            mbedtls_sha256_free(&sha);

            if (errbuf[0]) {
                Update.abort();
                show_error_screen(errbuf);
                delay(4000);
                draw_scan_card();
                break;
            }

            if (!Update.end()) {
                show_error_screen("OTA finalize failed");
                delay(4000);
                draw_scan_card();
                break;
            }

            if (!Update.isFinished()) {
                show_error_screen("OTA incomplete");
                delay(4000);
                draw_scan_card();
                break;
            }

            char sha_hex[65];
            for (int i = 0; i < 32; ++i) {
                snprintf(sha_hex + (i * 2), 3, "%02x", sha_bin[i]);
            }
            sha_hex[64] = '\0';
            Serial.printf("[OTA] Downloaded sha256=%s\n", sha_hex);

            // case-insensitive compare
            auto hex_eq_ci = [](const char* a, const char* b) -> bool {
                if (!a || !b) return false;
                while (*a && *b) {
                    char ca = *a++;
                    char cb = *b++;
                    if (ca >= 'A' && ca <= 'F') ca = (char)(ca - 'A' + 'a');
                    if (cb >= 'A' && cb <= 'F') cb = (char)(cb - 'A' + 'a');
                    if (ca != cb) return false;
                }
                return *a == '\0' && *b == '\0';
            };

            if (!hex_eq_ci(sha_hex, expected_sha256)) {
                show_error_screen("Checksum mismatch");
                delay(4500);
                draw_scan_card();
                break;
            }

            clear_screen();
            draw_title(_machine_name.c_str());
            draw_center_background(60, 120, 60);
            printTFTcentered("Restarting...", _tft->color565(255, 255, 255), 2, 0, 80, 320, 30);
            delay(1000);
            ESP.restart();
        }

        case Event::EVENT_CARD:
            char errbuf[64] = {0};
            if (g_network.api && g_network.api->start_session(g_last_scanned_access_key, &g_current_session, errbuf, sizeof(errbuf))) {
                g_hardware.relay_on();
                reset_session_active_usage_tracking();
                draw_machine_usage();
            } else {
                const char* err = errbuf[0] ? errbuf : "Start session failed";
                bool no_session = (strstr(errbuf, "no session") != nullptr) || (strstr(errbuf, "No session") != nullptr);
                if (no_session) {
                    _book_session_minutes = book_session_min_minutes;
                    draw_book_session();
                } else {
                    show_error_screen(err);
                    delay(5000);
                    draw_scan_card();
                }
            }
            break;
    }
}
